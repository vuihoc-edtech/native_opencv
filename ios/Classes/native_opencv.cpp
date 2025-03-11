#include <opencv2/opencv.hpp>
#include <chrono>
#include <optional>
#include <vector>
#include <string>
#include <algorithm>
#include "cjson/cJSON.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#define IS_WIN32
#endif

#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef IS_WIN32
#include <windows.h>
#endif

#if defined(__GNUC__)
#define FUNCTION_ATTRIBUTE __attribute__((visibility("default"))) __attribute__((used))
#elif defined(_MSC_VER)
#define FUNCTION_ATTRIBUTE __declspec(dllexport)
#endif

#define DRAW_USER_CHOICE true
#define DRAW_CHOICE_COLOR Scalar(255, 0, 0)

#define DRAW_BOXES false
#define DRAW_CHOICE_POINT false

#define DRAW_CIRCLE_RADIUS 6
#define DRAW_CIRCLE_THICKNESS -1

using namespace cv;
using namespace std;

// Function to get the current time in milliseconds
long long int get_now() {
    return chrono::duration_cast<std::chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch())
            .count();
}

// Platform-specific logging
void platform_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
#ifdef __ANDROID__
    __android_log_vprint(ANDROID_LOG_VERBOSE, "ndk", fmt, args);
#elif defined(IS_WIN32)
    char *buf = new char[4096];
    std::fill_n(buf, 4096, '\0');
    _vsprintf_p(buf, 4096, fmt, args);
    OutputDebugStringA(buf);
    delete[] buf;
#else
    vprintf(fmt, args);
#endif
    va_end(args);
}

// ___________________________
// Define bounding box and answer structs for better code organization
unordered_map<int, string> choicePart1 = {{1, "A"},
                                          {2, "B"},
                                          {3, "C"},
                                          {4, "D"}};
unordered_map<int, string> subQuestionPart2 = {{1, "a"},
                                               {2, "b"},
                                               {3, "c"},
                                               {4, "d"}};
unordered_map<int, string> choicePart2 = {{1, "yes"},
                                          {2, "no"}};
unordered_map<int, string> subChoicePart3 = {{1,  "-"},
                                             {2,  ","},
                                             {3,  "0"},
                                             {4,  "1"},
                                             {5,  "2"},
                                             {6,  "3"},
                                             {7,  "4"},
                                             {8,  "5"},
                                             {9,  "6"},
                                             {10, "7"},
                                             {11, "8"},
                                             {12, "9"}};
unordered_map<string, int> subChoicePart3Reversed = {{"-", 1},
                                                     {",", 2},
                                                     {"0", 3},
                                                     {"1", 4},
                                                     {"2", 5},
                                                     {"3", 6},
                                                     {"4", 7},
                                                     {"5", 8},
                                                     {"6", 9},
                                                     {"7", 10},
                                                     {"8", 11},
                                                     {"9", 12}};
unordered_map<int, string> choicePart3 = {{1, "1"},
                                          {2, "2"},
                                          {3, "3"},
                                          {4, "4"}};

struct OptionalPoint {
    bool hasValue;
    Point value;
};

struct part1Answer {
    string questionNumber;
    string userChoiceResult;
};

struct part2Answer {
    string questionNumber;
    string subName;
    bool userChoiceResult;
};

struct part3Answer {
    string questionNumber;
    string userResult;
};


// Resize the image to a fixed height and calculate the target width
Mat resizeImage(const Mat &image, int targetHeight) {
    double aspectRatio = static_cast<double>(image.cols) / image.rows;
    int targetWidth = static_cast<int>(targetHeight * aspectRatio);
    Mat resizedImage;
    resize(image, resizedImage, Size(targetWidth, targetHeight), 0, 0, INTER_AREA);
    return resizedImage;
}

// Hàm xoay ảnh dựa trên các đường thẳng đứng
Mat rotateImageVertically(const Mat& inputImage, double minLengthPercentage = 20.0) {
    // 1. Chuyển đổi sang ảnh xám
    Mat gray;
    cvtColor(inputImage, gray, COLOR_BGR2GRAY);

    // 2. Làm mờ ảnh
    Mat blurred;
    GaussianBlur(gray, blurred, Size(5, 5), 0);

    // 3. Phát hiện biên cạnh
    Mat edges;
    Canny(blurred, edges, 50, 150, 3);

    // 4. Tìm các đường thẳng
    vector<Vec4i> lines;
    HoughLinesP(edges, lines, 1, CV_PI / 180, 50, 50, 10);

    // 5. Tính toán góc xoay trung bình
    double angle = 0.0;
    int numLines = 0;

    // Tính toán minLength dựa trên tỷ lệ phần trăm chiều rộng ảnh
    double minLength = (minLengthPercentage / 100.0) * inputImage.cols;

    for (const auto& line : lines) {
        int x1 = line[0];
        int y1 = line[1];
        int x2 = line[2];
        int y2 = line[3];

        // Tính toán độ dài đường thẳng
        double length = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));

        // Lọc theo độ dài và hướng của đường thẳng (chọn đường thẳng đứng)
        if (length >= minLength && abs(y2 - y1) > abs(x2 - x1)) {
            double currentAngle = atan2(y2 - y1, x2 - x1) * 180.0 / CV_PI;

            // Điều chỉnh góc
            if (currentAngle > 0) {
                currentAngle -= 90.0;
            } else {
                currentAngle += 90.0;
            }

            angle += currentAngle;
            numLines++;
        }
    }
    if (numLines > 0) {
        angle /= numLines;
    }

    // Nếu góc xoay quá nhỏ, có thể coi là 0
    if (abs(angle) < 0.2) {
        angle = 0.0;
    }


    // 6. Xoay ảnh
    Point2f center(inputImage.cols / 2.0, inputImage.rows / 2.0);
    Mat rotationMatrix = getRotationMatrix2D(center, angle, 1.0);
    Mat rotatedImage;
    warpAffine(inputImage, rotatedImage, rotationMatrix, inputImage.size(), INTER_LINEAR, BORDER_TRANSPARENT, Scalar(0, 0, 0));

    return rotatedImage;
}

// Hàm xoay ảnh dựa trên các đường thẳng nằm ngang
Mat rotateImageHorizontally(const Mat& inputImage, double minLengthPercentage = 20.0) {
    // 1. Chuyển đổi sang ảnh xám
    Mat gray;
    cvtColor(inputImage, gray, COLOR_BGR2GRAY);

    // 2. Làm mờ ảnh
    Mat blurred;
    GaussianBlur(gray, blurred, Size(5, 5), 0);

    // 3. Phát hiện biên cạnh
    Mat edges;
    Canny(blurred, edges, 50, 150, 3);

    // 4. Tìm các đường thẳng
    vector<Vec4i> lines;
    HoughLinesP(edges, lines, 1, CV_PI / 180, 50, 50, 10);

    // 5. Tính toán góc xoay trung bình
    double angle = 0.0;
    int numLines = 0;

    // Tính toán minLength dựa trên tỷ lệ phần trăm chiều cao ảnh
    double minLength = (minLengthPercentage / 100.0) * inputImage.rows;

    for (const auto& line : lines) {
        int x1 = line[0];
        int y1 = line[1];
        int x2 = line[2];
        int y2 = line[3];

        // Tính toán độ dài đường thẳng
        double length = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));

        // Lọc theo độ dài và hướng của đường thẳng (chọn đường nằm ngang)
        if (length >= minLength && abs(y2 - y1) < abs(x2 - x1)) {
            double currentAngle = atan2(y2 - y1, x2 - x1) * 180.0 / CV_PI;
            angle += currentAngle;
            numLines++;
        }
    }
    if (numLines > 0) {
        angle /= numLines;
    }

    // Nếu góc xoay quá nhỏ, có thể coi là 0
    if (abs(angle) < 0.2) {
        angle = 0.0;
    }

    // 6. Xoay ảnh
    Point2f center(inputImage.cols / 2.0, inputImage.rows / 2.0);
    Mat rotationMatrix = getRotationMatrix2D(center, angle, 1.0);
    Mat rotatedImage;
    warpAffine(inputImage, rotatedImage, rotationMatrix, inputImage.size(), INTER_LINEAR, BORDER_TRANSPARENT, Scalar(0, 0, 0));

    return rotatedImage;
}

// Hàm so sánh cho việc sắp xếp contours
struct ContourPrecedenceComparator {
    int cols;
    ContourPrecedenceComparator(int c) : cols(c) {}

    bool operator()(const vector<Point>& c1, const vector<Point>& c2) const {
        int tolerance_factor = 60;
        Rect origin1 = boundingRect(c1);
        Rect origin2 = boundingRect(c2);
        return ((origin1.y / tolerance_factor) * tolerance_factor) * cols + origin1.x <
               ((origin2.y / tolerance_factor) * tolerance_factor) * cols + origin2.x;
    }
};

// Image preprocessing: Convert to grayscale, blur, and apply adaptive thresholding
Mat preprocessOriginImage(const Mat &image) {
    Mat gray, claheImage, blurred, sharpenedImage, thresh, dilated, closed;
    cvtColor(image, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, blurred, Size(5, 5), 0);
    Ptr<CLAHE> clahe = createCLAHE();
    clahe->setClipLimit(2.0);
    clahe->setTilesGridSize(Size(8, 8));
    clahe->apply(blurred, claheImage);
    adaptiveThreshold(claheImage, thresh, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 21, 15);
    Mat kernel = getStructuringElement(MORPH_RECT, Size(2, 2));
    dilate(thresh, dilated, kernel, Point(-1, -1), 1);
    morphologyEx(dilated, closed, MORPH_CLOSE, kernel);
    return closed;
}

bool areBoxSimilar(const Rect& box1, const Rect& box2, int threshold = 10) {
    int x1 = box1.x;
    int y1 = box1.y;
    int w1 = box1.width;
    int h1 = box1.height;

    int x2 = box2.x;
    int y2 = box2.y;
    int w2 = box2.width;
    int h2 = box2.height;

    // Check if the boxes are close in size
    bool sizeCondition = (abs(w1 - w2) < threshold) && (abs(h1 - h2) < threshold);

    // Check if the boxes overlap or are close in position
    bool positionCondition = (abs(x1 - x2) < threshold) && (abs(y1 - y2) < threshold);

    return sizeCondition && positionCondition;
}

// Processing part 3
Mat preprocessPart3(const Mat& image) {
    // Chuyển ảnh sang ảnh xám
    Mat gray;
    cvtColor(image, gray, COLOR_BGR2GRAY);

    // Làm mờ ảnh để giảm nhiễu
    Mat blurred;
    GaussianBlur(gray, blurred, Size(5, 5), 0);

    // Áp dụng adaptive thresholding
    Mat thresh;
    adaptiveThreshold(blurred, thresh, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 21, 15);

    // Áp dụng phép giãn nở (dilate) để nối các đường nét bị đứt
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    Mat dilated;
    dilate(thresh, dilated, kernel, Point(-1, -1), 1);  // iterations = 1


    // Áp dụng phép đóng (closing) để lấp đầy các lỗ hổng nhỏ
    Mat closed;
    morphologyEx(dilated, closed, MORPH_CLOSE, kernel);

    // Áp dụng Canny Edge Detection
    Mat edges;
    Canny(closed, edges, 50, 150, 3); // apertureSize = 3

    return edges;
}

vector<Rect> findContoursOrigin(const Mat& image) {
    vector <vector<Point>> contours;
    findContours(image, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector <Rect> boundingBoxes;

    // Sắp xếp contours
    sort(contours.begin(), contours.end(), ContourPrecedenceComparator(image.cols));

    for (const auto &contour: contours) {
        if (contourArea(contour) < 1000) continue;
        vector <Point> approx;
        double epsilon = 0.04 * arcLength(contour, true);
        approxPolyDP(contour, approx, epsilon, true);
        if (approx.size() == 4) { // Kiểm tra hình chữ nhật
            Rect boundRect = boundingRect(approx);
            if (boundRect.y > image.rows * 2 / 11) {
                boundingBoxes.push_back(
                        boundRect);
            }
        }
    }
    return boundingBoxes;
}


vector<Rect> findContoursPart3(const Mat& image) {
    vector<vector<Point>> contours;
    findContours(image, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

    // Sắp xếp contours
    sort(contours.begin(), contours.end(), ContourPrecedenceComparator(image.cols));


    vector<Rect> boundingBoxes;
    for (const auto& contour : contours) {
        double area = contourArea(contour);
        if (area < 20000 || area > 50000) {
            continue;
        }


        double perimeter = arcLength(contour, true);
        vector<Point> approx;
        approxPolyDP(contour, approx, 0.04 * perimeter, true);

        Rect boundRect = boundingRect(approx);


        double aspectRatio = (double)boundRect.width / boundRect.height;
        
        if (aspectRatio < 0.35 || aspectRatio > 0.45) {
            continue;
        }

        if (approx.size() == 4) {
            boundingBoxes.push_back(boundingRect(approx));
        }
    }

    vector<Rect> mergedBoxes;
    for (const auto& box : boundingBoxes) {
        bool merged = false;
        for (size_t i = 0; i < mergedBoxes.size(); ++i) {
            if (areBoxSimilar(box, mergedBoxes[i])) {
                Rect& existing_box = mergedBoxes[i]; //  Lấy reference để sửa trực tiếp

                int newX = min(existing_box.x, box.x);
                int newY = min(existing_box.y, box.y);
                int newWidth = max(existing_box.x + existing_box.width, box.x + box.width) - newX;
                int newHeight = max(existing_box.y + existing_box.height, box.y + box.height) - newY;


                mergedBoxes[i] = Rect(newX, newY, newWidth, newHeight);
                merged = true;
                break;
            }
        }
        if (!merged) {
            mergedBoxes.push_back(box);
        }
    }

    return mergedBoxes;
}


// Find and filter contours based on area and height, returning bounding boxes
vector <Rect> extractBoundingBoxes(const Mat &originalImage) {

    // Tiền xử lý ảnh
    Mat processedImage = preprocessOriginImage(originalImage.clone());


    // Tìm contours và bounding boxes
    vector <Rect> boundingBoxes = findContoursOrigin(processedImage);


     // Tách part 3 thành các box riêng biệt
    if (boundingBoxes.size() > 8) { // Kiểm tra xem có đủ bounding boxes không
        Rect box8 = boundingBoxes[8];

        // Sử dụng Rect để dễ dàng crop ảnh
        Rect roi(max(0, box8.x - 10), max(0, box8.y - 10), 
                    min(originalImage.cols - (box8.x - 10), box8.width + 20),
                    min(originalImage.rows - (box8.y - 10), box8.height + 20));

        Mat cropImage8 = originalImage(roi);


        cropImage8 = preprocessPart3(cropImage8);
        vector<Rect> boundingBoxesPart3 = findContoursPart3(cropImage8);


        // Xóa bounding box cũ của part 3
        boundingBoxes.erase(boundingBoxes.begin() + 8);


        // Thêm các bounding box mới, đã điều chỉnh tọa độ
        for (const auto& box : boundingBoxesPart3) {
            boundingBoxes.push_back(Rect(box.x + roi.x, box.y + roi.y, box.width, box.height));
        }

        // Kiểm ra aspect ratio của bounding box
        for (const auto& box : boundingBoxes) {
            double aspectRatio = (double)box.width / box.height;
            if (aspectRatio < 0.35 || aspectRatio > 1.5) {
                // Xóa bounding box không hợp lệ
                boundingBoxes.erase(remove(boundingBoxes.begin(), boundingBoxes.end(), box), boundingBoxes.end());
            }
        }
    }
    return boundingBoxes;
}

bool isContourFilled(const vector<Point>& contour, const Mat& image_threshold) {
    Rect boundingBox = boundingRect(contour);
    Mat image_roi = image_threshold(boundingBox);
    double filledArea = countNonZero(image_roi);
    return filledArea / (boundingBox.width*boundingBox.height) > 0.5;
}

// Check for a circular mark indicating an answer in the choice image
OptionalPoint detectChoiceCircle(const Mat &choiceImage, int binaryThreshold = 200) {
    Mat gray, thresh;
    cvtColor(choiceImage, gray, COLOR_BGR2GRAY);
    // threshold(blurred, thresh, 220, 255, THRESH_BINARY_INV);
    adaptiveThreshold(gray, thresh, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 21, 15);
    vector <vector<Point>> contours;
    findContours(thresh, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    for (const auto &contour: contours) {
        if (contourArea(contour) < 30) continue;
        Rect boundingBox = boundingRect(contour);
        double aspectRatio = 0.0;
        aspectRatio = static_cast<double>(boundingBox.width) / boundingBox.height;
        if (aspectRatio < 1.5 && aspectRatio > 0.5) {
            Point2f center;
            float radius;
            minEnclosingCircle(contour, center, radius);
            if(isContourFilled(contour, thresh)) {
                return {true, Point(static_cast<int>(center.x), static_cast<int>(center.y))};
            }
            else{
                return {false, Point(static_cast<int>(center.x), static_cast<int>(center.y))};
            }
        }
    }
    return {false, Point(0, 0)};
}

int findCorrectIndex(const vector<int>& correctChoices) {
    for (int i = 0; i < correctChoices.size(); i++) {
        if (correctChoices[i] == 1) {
            return i;
        }
    }
    return -1;
}

// Function to parse correct answers from JSON string
void parseCorrectAnswers(const char* json, vector<vector<int>>& part1CorrectChoices, vector<vector<vector<int>>>& part2CorrectChoices, vector<vector<vector<int>>>& part3CorrectChoices) {
    // Đọc chuỗi JSON chứa đáp án đúng
    string jsonCorrectString(json);
    cJSON *jsonParse = cJSON_Parse(jsonCorrectString.c_str());
    cJSON *correctAnswersJson = cJSON_GetObjectItem(jsonParse, "answers");
    cJSON *correctPart1Json = cJSON_GetObjectItem(correctAnswersJson, "1");
    cJSON *correctPart2Json = cJSON_GetObjectItem(correctAnswersJson, "2");
    cJSON *correctPart3Json = cJSON_GetObjectItem(correctAnswersJson, "3");

    // Part 1
    cJSON *questionPart1 = correctPart1Json->child;
    while (questionPart1 != NULL) {
        const char *key = questionPart1->string;
        const char *value = questionPart1->valuestring;
        part1CorrectChoices[atoi(key) - 1][value[0] - 'A'] = 1;
        questionPart1 = questionPart1->next;
    }

    // Part 2
    cJSON* questionPart2 = correctPart2Json->child;
    while (questionPart2 != nullptr) {
        int questionNumber = atoi(questionPart2->string);
        cJSON* subquestion = questionPart2->child;
        while (subquestion != nullptr) {
            const char* subName = subquestion->string;
            bool correctChoice = cJSON_IsTrue(subquestion);
            part2CorrectChoices[questionNumber - 1][subName[0] - 'a'][!correctChoice] = 1;
            subquestion = subquestion->next;
        }
        questionPart2 = questionPart2->next;
    }

    // Part 3
    cJSON* questionPart3 = correctPart3Json->child;
    while (questionPart3 != nullptr) {
        int questionNumber = atoi(questionPart3->string);
        const char* correctResult = questionPart3->valuestring;
        int index = 0;
        for (char c : string(correctResult)) {
            int choiceIndex = subChoicePart3Reversed[string(1, c)];
            part3CorrectChoices[questionNumber - 1][choiceIndex - 1][index] = 1;
            index++;
        }
        questionPart3 = questionPart3->next;
    }

    cJSON_Delete(jsonParse);
}



// ___________________________
// Avoiding name mangling for cross-platform compatibility
extern "C"
{

FUNCTION_ATTRIBUTE
const char *version() {
    return CV_VERSION;
}

// Main function for processing the image
FUNCTION_ATTRIBUTE
const char *process_image(const char *imgPath, const char *outputPath, const char * json) {
    // Initialize vectors to store correct choices for each part
    // vector<vector<int>> part1CorrectChoices(40, vector<int>(4, 0));
    // vector<vector<vector<int>>> part2CorrectChoices(8, vector<vector<int>>(4, vector<int>(2, 0)));
    // vector<vector<vector<int>>> part3CorrectChoices(6, vector<vector<int>>(12, vector<int>(4, 0)));

    // Parse the correct answers from the provided JSON string
    // parseCorrectAnswers(json, part1CorrectChoices, part2CorrectChoices, part3CorrectChoices);

    // Create a JSON object to store the results
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "version", "15");
    cJSON *answersJson = cJSON_AddObjectToObject(root, "answers");

    // Đọc ảnh từ đường dẫn
    Mat originalImage = imread(imgPath);

    if (originalImage.empty()) {
        cJSON_AddNumberToObject(root, "status_code", 1);
        cJSON_AddStringToObject(root, "error", "Image not found");
    
        // Chuyển object JSON thành chuỗi
        char* jsonString = cJSON_Print(root);
        
        // Cấp phát bộ nhớ động cho chuỗi JSON (nhớ giải phóng sau khi sử dụng)
        char* jsonResult = (char*)malloc(strlen(jsonString) + 1);
        strcpy(jsonResult, jsonString);
        
        // Giải phóng bộ nhớ
        cJSON_Delete(root);
        free(jsonString); // Giải phóng chuỗi cJSON tạo ra
        
        return jsonResult;

    }
    // Rotate the image
    originalImage = rotateImageVertically(originalImage.clone(), 20.0);
    originalImage = rotateImageHorizontally(originalImage.clone(), 20.0);

    // Resize the image to a fixed height
    originalImage = resizeImage(originalImage, 1280);
    


    Mat outputImage = originalImage.clone();

    vector <Rect> boundingBoxes;
    try {
        // Extract bounding boxes from the image
        boundingBoxes = extractBoundingBoxes(originalImage);
        
        // Vẽ bounding boxes lên ảnh
        if (DRAW_BOXES) {
            int count = 1;
            for (const auto &box: boundingBoxes) {
                rectangle(outputImage, Point(box.x, box.y), Point(box.x + box.width, box.y + box.height),
                        Scalar(0, 255, 0), 2);
                putText(outputImage, to_string(count), Point(box.x, box.y), FONT_HERSHEY_SIMPLEX, 1,
                        Scalar(0, 255, 0), 2);
                count++;
            }
        }
        if(boundingBoxes.size() != 14) {
            throw runtime_error("Found " + to_string(boundingBoxes.size()) + " bounding boxes, expected 14");
        }
    } catch (const exception &e) {
        cJSON_AddNumberToObject(root, "status_code", 1);
        cJSON_AddStringToObject(root, "error", e.what());
        // Chuyển object JSON thành chuỗi
        char* jsonString = cJSON_Print(root);
        
        // Cấp phát bộ nhớ động cho chuỗi JSON (nhớ giải phóng sau khi sử dụng)
        char* jsonResult = (char*)malloc(strlen(jsonString) + 1);
        strcpy(jsonResult, jsonString);
        
        // Giải phóng bộ nhớ
        cJSON_Delete(root);
        free(jsonString); // Giải phóng chuỗi cJSON tạo ra
        
        return jsonResult;
    }

    // Part 1
    vector<part1Answer> part1Answers;
    try {
        vector<vector<int>> part1UserChoices(40, vector<int>(4, 0));
        vector<vector<Point>> part1ChoicePoints(40, vector<Point>(4, Point(0, 0))); 
        for (int boundingBoxIndex = 0; boundingBoxIndex < min(4, static_cast<int>(boundingBoxes.size())); boundingBoxIndex++) {
            Rect bbox = boundingBoxes[boundingBoxIndex];
            int yOffset = bbox.height * 0.09;
            int xOffset = bbox.width * 0.2;
            int yPadding = bbox.height*0.095, xPadding = bbox.width*0.154;
            for (int rowIndex = 0; rowIndex < 10; rowIndex++) {
                bool missingChoiceFlag = true;
                bool choiceIsCorrectFlag = true;
                for (int colIndex = 0; colIndex < 4; colIndex++) {
                    int linearIndex = boundingBoxIndex * 10 + rowIndex;
                    int x = bbox.x + xPadding + xOffset * colIndex + 1;
                    int y = bbox.y + yPadding + yOffset * rowIndex + 1;
                    int width = xOffset + 1;
                    int height = yOffset + 1;
                    Mat choiceRegion = originalImage(Rect(x, y, width, height));
                    OptionalPoint detectedCircle = detectChoiceCircle(choiceRegion, 200);
                    // bool isCorrect = part1CorrectChoices[linearIndex][colIndex] == 1;
                    if(detectedCircle.hasValue) {
                        part1Answer answer = {to_string(boundingBoxIndex*10 + rowIndex + 1), choicePart1[colIndex + 1]};
                        part1Answers.push_back(answer);
                        part1UserChoices[linearIndex][colIndex] = 1;
                        part1ChoicePoints[linearIndex][colIndex] = Point{x + detectedCircle.value.x, y + detectedCircle.value.y};
                        if (DRAW_USER_CHOICE) {
                            circle(outputImage, Point{x + detectedCircle.value.x, y + detectedCircle.value.y},
                                    DRAW_CIRCLE_RADIUS,
                                    DRAW_CHOICE_COLOR, DRAW_CIRCLE_THICKNESS);
                        }
                    }
                    else{
                        part1UserChoices[linearIndex][colIndex] = 0;
                        if(detectedCircle.value.x!=0 && detectedCircle.value.y!=0){
                            part1ChoicePoints[linearIndex][colIndex] = Point{x + detectedCircle.value.x, y + detectedCircle.value.y};
                        }
                        else{
                            part1ChoicePoints[linearIndex][colIndex] = Point{x + width / 2, y + height / 2};
                        }
            
                    }
                }

            }
        }
    }
    catch (const exception &e) {
        cJSON_AddNumberToObject(root, "status_code", 1);
        cJSON_AddStringToObject(root, "error", e.what());
        // Chuyển object JSON thành chuỗi
        char* jsonString = cJSON_Print(root);
        
        // Cấp phát bộ nhớ động cho chuỗi JSON (nhớ giải phóng sau khi sử dụng)
        char* jsonResult = (char*)malloc(strlen(jsonString) + 1);
        strcpy(jsonResult, jsonString);
        
        // Giải phóng bộ nhớ
        cJSON_Delete(root);
        free(jsonString); // Giải phóng chuỗi cJSON tạo ra
        
        return jsonResult;
    }
    

    // Part 2
    vector<part2Answer> part2Answers;
    try{
        vector<vector<vector<int>>> part2UserChoices(8, vector<vector<int>>(4, vector<int>(2, 0)));
        vector<vector<vector<Point>>> part2ChoicePoints(8, vector<vector<Point>>(4, vector<Point>(2, Point(0, 0))));
        for (int boundingBoxIndex = 4; boundingBoxIndex < 8 && boundingBoxIndex < boundingBoxes.size(); boundingBoxIndex++) {
            Rect bbox = boundingBoxes[boundingBoxIndex];
            int yOffset = bbox.height * 0.15;
            int xOffset = bbox.width * 0.21;
            int yPadding = bbox.height*0.347, xPadding = bbox.width*0.123;
            int blockIndex = boundingBoxIndex - 4;
            for (int rowIndex = 0; rowIndex < 4; rowIndex++) {
                for (int colIndex = 0; colIndex < 4; colIndex++) {
                    int x = bbox.x + xPadding + xOffset * colIndex + 1;
                    int y = bbox.y + yPadding + yOffset * rowIndex + 1;
                    int width = xOffset + 1;
                    int height = yOffset + 1;
                    Mat choiceRegion = originalImage(Rect(x, y, width, height));
                    int questionNumberIndex = blockIndex * 2 + colIndex / 2;
                    OptionalPoint detectedCircle = detectChoiceCircle(choiceRegion, 200);

                    int choiceIndex = colIndex % 2;
                    if (detectedCircle.hasValue) {
                        part2UserChoices[questionNumberIndex][rowIndex][choiceIndex] = 1;
                        part2ChoicePoints[questionNumberIndex][rowIndex][choiceIndex] = Point{x + detectedCircle.value.x, y + detectedCircle.value.y};
                        part2Answer answer = {to_string(questionNumberIndex + 1), subQuestionPart2[rowIndex + 1], choiceIndex==0};
                        part2Answers.push_back(answer);
                        if (DRAW_USER_CHOICE) {
                            circle(outputImage, Point{x + detectedCircle.value.x, y + detectedCircle.value.y},
                                    DRAW_CIRCLE_RADIUS,
                                    DRAW_CHOICE_COLOR, DRAW_CIRCLE_THICKNESS);
                        }
                    } else{
                        part2UserChoices[questionNumberIndex][rowIndex][choiceIndex] = 0;
                        if(detectedCircle.value.x!=0 && detectedCircle.value.y!=0){
                            part2ChoicePoints[questionNumberIndex][rowIndex][choiceIndex] = Point{x + detectedCircle.value.x, y + detectedCircle.value.y};
                        }
                        else{
                            part2ChoicePoints[questionNumberIndex][rowIndex][choiceIndex] = Point{x + width / 2, y + height / 2};
                        }
                    }
                }        
            }
        }

        // Sort the answers by question number and sub name
        sort(part2Answers.begin(), part2Answers.end(), [](const part2Answer &a, const part2Answer &b)
            {
        if (a.questionNumber != b.questionNumber) {
            return a.questionNumber < b.questionNumber;
        }
        return a.subName < b.subName; });
    }
    catch (const exception &e) {
        cJSON_AddNumberToObject(root, "status_code", 1);
        cJSON_AddStringToObject(root, "error", e.what());
        // Chuyển object JSON thành chuỗi
        char* jsonString = cJSON_Print(root);
        
        // Cấp phát bộ nhớ động cho chuỗi JSON (nhớ giải phóng sau khi sử dụng)
        char* jsonResult = (char*)malloc(strlen(jsonString) + 1);
        strcpy(jsonResult, jsonString);
        
        // Giải phóng bộ nhớ
        cJSON_Delete(root);
        free(jsonString); // Giải phóng chuỗi cJSON tạo ra
        
        return jsonResult;
    }

    //Part 3
    vector<part3Answer> part3Answers;
    try {
        vector<vector<vector<int>>> part3UserChoices(6, vector<vector<int>>(12, vector<int>(4, 0)));
        vector<vector<vector<Point>>> part3ChoicePoints(6, vector<vector<Point>>(12, vector<Point>(4, Point(0, 0))));
        for (int boundingBoxIndex = 8; boundingBoxIndex < min(14, static_cast<int>(boundingBoxes.size())); boundingBoxIndex++) {
            Rect bbox = boundingBoxes[boundingBoxIndex];
            int yOffset = bbox.height * 0.07;
            int xOffset = bbox.width * 0.19;
            int yPadding = bbox.height*0.1625, xPadding = bbox.width*0.18;
            int blockIndex = boundingBoxIndex - 8;
            string questionNumber = to_string(blockIndex + 1);
            string userResult = "";
            for (int colIndex = 0; colIndex < 4; colIndex++) {
                for (int rowIndex = 0; rowIndex < 12; rowIndex++) {
                    int x = bbox.x + xPadding + xOffset * colIndex + 1;
                    int y = bbox.y + yPadding + yOffset * rowIndex + 1;
                    int width = xOffset + 1;
                    int height = yOffset + 1;

                    Mat choiceRegion = originalImage(Rect(x, y, width, height));
                    OptionalPoint detectedCircle = detectChoiceCircle(choiceRegion, 200);
                    // bool isCorrect = part3CorrectChoices[blockIndex][rowIndex][colIndex] == 1;

                    if (detectedCircle.hasValue) {
                        part3UserChoices[blockIndex][rowIndex][colIndex] = 1;
                        part3ChoicePoints[blockIndex][rowIndex][colIndex] = Point{x + detectedCircle.value.x, y + detectedCircle.value.y};
                        userResult += subChoicePart3[rowIndex + 1];
                        if (DRAW_USER_CHOICE) {
                            circle(outputImage, Point{x + detectedCircle.value.x, y + detectedCircle.value.y},
                                    DRAW_CIRCLE_RADIUS,
                                    DRAW_CHOICE_COLOR, DRAW_CIRCLE_THICKNESS);
                        }

                    } else{
                        part3UserChoices[blockIndex][rowIndex][colIndex] = 0;
                        if(detectedCircle.value.x!=0 && detectedCircle.value.y!=0){
                            part3ChoicePoints[blockIndex][rowIndex][colIndex] = Point{x + detectedCircle.value.x, y + detectedCircle.value.y};
                        }
                        else{
                            part3ChoicePoints[blockIndex][rowIndex][colIndex] = Point{x + width / 2, y + height / 2};
                        }
                    }
                }
            }
            if (!userResult.empty()) {
                part3Answer answer = {questionNumber, userResult};
                part3Answers.push_back(answer);
            }
        }
    }
    catch (const exception &e) {
        cJSON_AddNumberToObject(root, "status_code", 1);
        cJSON_AddStringToObject(root, "error", e.what());
        // Chuyển object JSON thành chuỗi
        char* jsonString = cJSON_Print(root);
        
        // Cấp phát bộ nhớ động cho chuỗi JSON (nhớ giải phóng sau khi sử dụng)
        char* jsonResult = (char*)malloc(strlen(jsonString) + 1);
        strcpy(jsonResult, jsonString);
        
        // Giải phóng bộ nhớ
        cJSON_Delete(root);
        free(jsonString); // Giải phóng chuỗi cJSON tạo ra
        
        return jsonResult;
    }
    if (part1Answers.size() + part2Answers.size() + part3Answers.size() == 0) {
        cJSON_AddNumberToObject(root, "status_code", 2);
        cJSON_AddStringToObject(root, "error", "No answers detected");
        // Chuyển object JSON thành chuỗi
        char* jsonString = cJSON_Print(root);
        
        // Cấp phát bộ nhớ động cho chuỗi JSON (nhớ giải phóng sau khi sử dụng)
        char* jsonResult = (char*)malloc(strlen(jsonString) + 1);
        strcpy(jsonResult, jsonString);
        
        // Giải phóng bộ nhớ
        cJSON_Delete(root);
        free(jsonString); // Giải phóng chuỗi cJSON tạo ra
        
        return jsonResult;
    }

    cJSON *part1ResultJson = cJSON_AddObjectToObject(answersJson, "1");
    cJSON *part2ResultJson = cJSON_AddObjectToObject(answersJson, "2");
    cJSON *part3ResultJson = cJSON_AddObjectToObject(answersJson, "3");

    for (const auto &answer: part1Answers) {
        cJSON_AddStringToObject(part1ResultJson, answer.questionNumber.c_str(), answer.userChoiceResult.c_str());
    }

    for (const auto& answer : part2Answers) {
        cJSON *questionObj = cJSON_GetObjectItemCaseSensitive(part2ResultJson, answer.questionNumber.c_str());
        if (questionObj == nullptr) {
            questionObj = cJSON_CreateObject();
            cJSON_AddItemToObject(part2ResultJson, answer.questionNumber.c_str(), questionObj);
        }

        cJSON *boolValue = cJSON_CreateBool(answer.userChoiceResult);
        cJSON_AddItemToObject(questionObj, answer.subName.c_str(), boolValue);
    }

    for (const auto &answer: part3Answers) {
        cJSON_AddStringToObject(part3ResultJson, answer.questionNumber.c_str(), answer.userResult.c_str());
    }

    
    if (!imwrite(outputPath, outputImage)) {
        cJSON_AddNumberToObject(root, "status_code", 1);
        cJSON_AddStringToObject(root, "error", "Failed to save output image");
        // Chuyển object JSON thành chuỗi
        char* jsonString = cJSON_Print(root);
        
        // Cấp phát bộ nhớ động cho chuỗi JSON (nhớ giải phóng sau khi sử dụng)
        char* jsonResult = (char*)malloc(strlen(jsonString) + 1);
        strcpy(jsonResult, jsonString);
        
        // Giải phóng bộ nhớ
        cJSON_Delete(root);
        free(jsonString); // Giải phóng chuỗi cJSON tạo ra
        
        return jsonResult;
    }

    cJSON_AddNumberToObject(root, "status_code", 0);
    // Chuyển object JSON thành chuỗi
    char* jsonString = cJSON_Print(root);
    
    // Cấp phát bộ nhớ động cho chuỗi JSON (nhớ giải phóng sau khi sử dụng)
    char* jsonResult = (char*)malloc(strlen(jsonString) + 1);
    strcpy(jsonResult, jsonString);
    
    // Giải phóng bộ nhớ
    cJSON_Delete(root);
    free(jsonString); // Giải phóng chuỗi cJSON tạo ra
    
    return jsonResult;
}
}
