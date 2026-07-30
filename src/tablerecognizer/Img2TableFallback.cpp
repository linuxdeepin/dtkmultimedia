// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Img2TableFallback.h"

#include <QDebug>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

namespace {

// QImage -> cv::Mat (BGR 8UC3)。
cv::Mat qImageToMat(const QImage &image)
{
    QImage rgb = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(rgb.height(), rgb.width(), CV_8UC3,
                const_cast<uchar *>(rgb.constBits()), static_cast<size_t>(rgb.bytesPerLine()));
    cv::Mat bgr;
    cv::cvtColor(mat, bgr, cv::COLOR_RGB2BGR);
    return bgr.clone();
}

// 提取近似水平的线（y 在阈值内变化）。
std::vector<cv::Vec4i> detectLines(const cv::Mat &mask, bool horizontal)
{
    cv::Mat edges;
    cv::Canny(mask, edges, 50, 150);
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, 1, CV_PI / 180, 50, 50, 10);
    std::vector<cv::Vec4i> result;
    for (const cv::Vec4i &l : lines) {
        const int dx = std::abs(l[0] - l[2]);
        const int dy = std::abs(l[1] - l[3]);
        if (horizontal && dx > 0 && dy <= 2 && dx > 30)
            result.push_back(l);
        else if (!horizontal && dy > 0 && dx <= 2 && dy > 30)
            result.push_back(l);
    }
    return result;
}

// 把坐标按阈值聚类，返回代表坐标。
std::vector<int> clusterCoords(std::vector<int> coords, int threshold)
{
    if (coords.empty())
        return {};
    std::sort(coords.begin(), coords.end());
    std::vector<int> out;
    out.push_back(coords.front());
    for (int c : coords) {
        if (c - out.back() > threshold)
            out.push_back(c);
    }
    return out;
}

} // namespace

bool Img2TableFallback::available() const
{
    return true;
}

bool Img2TableFallback::detect(const QImage &image, QList<DetectedCell> &cells, QString &error)
{
    cells.clear();
    if (image.isNull()) {
        error = QStringLiteral("输入图片无效");
        return false;
    }

    cv::Mat src = qImageToMat(image);
    if (src.empty()) {
        error = QStringLiteral("图片转换失败");
        return false;
    }

    cv::Mat gray;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    cv::Mat binary;
    cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

    // 水平线检测。
    cv::Mat horiz;
    cv::Mat horizKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(std::max(1, src.cols / 20), 1));
    cv::morphologyEx(binary, horiz, cv::MORPH_OPEN, horizKernel);
    auto hLines = detectLines(horiz, true);

    // 垂直线检测。
    cv::Mat vert;
    cv::Mat vertKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, std::max(1, src.rows / 20)));
    cv::morphologyEx(binary, vert, cv::MORPH_OPEN, vertKernel);
    auto vLines = detectLines(vert, false);

    if (hLines.size() < 2 || vLines.size() < 2) {
        error = QStringLiteral("未识别到表格线");
        return false;
    }

    // 行边界 = 水平线的 y 坐标聚类。
    std::vector<int> ys;
    for (const cv::Vec4i &l : hLines)
        ys.push_back((l[1] + l[3]) / 2);
    ys = clusterCoords(ys, std::max(2, src.rows / 40));

    std::vector<int> xs;
    for (const cv::Vec4i &l : vLines)
        xs.push_back((l[0] + l[2]) / 2);
    xs = clusterCoords(xs, std::max(2, src.cols / 40));

    if (ys.size() < 2 || xs.size() < 2) {
        error = QStringLiteral("表格线聚类后不足构成网格");
        return false;
    }

    for (size_t r = 0; r + 1 < ys.size(); ++r) {
        for (size_t c = 0; c + 1 < xs.size(); ++c) {
            DetectedCell cell;
            cell.row = int(r);
            cell.col = int(c);
            cell.rowSpan = 1;
            cell.colSpan = 1;
            cell.bbox = QRectF(QPointF(xs[c], ys[r]), QPointF(xs[c + 1], ys[r + 1]));
            cells.append(cell);
        }
    }
    return true;
}

D_TABLERECOGNIZER_END_NAMESPACE
