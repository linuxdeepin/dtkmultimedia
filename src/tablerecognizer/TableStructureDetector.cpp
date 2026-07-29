// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "TableStructureDetector.h"

#include "OrtInferenceEngine.h"

#include <QDebug>
#include <algorithm>
#include <QMap>
#include <QString>

D_TABLERECOGNIZER_BEGIN_NAMESPACE

namespace {

// SLANet_plus 结构词表（50 tokens）。
// 词表顺序与 PaddleOCR TableLabelDecode.add_special_char 一致：
//   [sos] + character_dict (48 entries) + [eos]
// character_dict 从 ONNX 模型元数据 character 字段提取。
const QStringList &vocab()
{
    static const QStringList v = {
        QStringLiteral("<sos>"),           // 0  — 序列起始（PaddleOCR beg_str）
        QStringLiteral("<thead>"),         // 1
        QStringLiteral("</thead>"),        // 2
        QStringLiteral("<tbody>"),         // 3
        QStringLiteral("</tbody>"),        // 4
        QStringLiteral("<tr>"),            // 5
        QStringLiteral("</tr>"),           // 6
        QStringLiteral("<td"),             // 7  — 单元格起始（需 > 闭合）
        QStringLiteral(">"),               // 8  — 闭合单元格属性
        QStringLiteral("</td>"),           // 9
        QStringLiteral(" colspan=\"2\""),  // 10
        QStringLiteral(" colspan=\"3\""),  // 11
        QStringLiteral(" colspan=\"4\""),  // 12
        QStringLiteral(" colspan=\"5\""),  // 13
        QStringLiteral(" colspan=\"6\""),  // 14
        QStringLiteral(" colspan=\"7\""),  // 15
        QStringLiteral(" colspan=\"8\""),  // 16
        QStringLiteral(" colspan=\"9\""),  // 17
        QStringLiteral(" colspan=\"10\""), // 18
        QStringLiteral(" colspan=\"11\""), // 19
        QStringLiteral(" colspan=\"12\""), // 20
        QStringLiteral(" colspan=\"13\""), // 21
        QStringLiteral(" colspan=\"14\""), // 22
        QStringLiteral(" colspan=\"15\""), // 23
        QStringLiteral(" colspan=\"16\""), // 24
        QStringLiteral(" colspan=\"17\""), // 25
        QStringLiteral(" colspan=\"18\""), // 26
        QStringLiteral(" colspan=\"19\""), // 27
        QStringLiteral(" colspan=\"20\""), // 28
        QStringLiteral(" rowspan=\"2\""),  // 29
        QStringLiteral(" rowspan=\"3\""),  // 30
        QStringLiteral(" rowspan=\"4\""),  // 31
        QStringLiteral(" rowspan=\"5\""),  // 32
        QStringLiteral(" rowspan=\"6\""),  // 33
        QStringLiteral(" rowspan=\"7\""),  // 34
        QStringLiteral(" rowspan=\"8\""),  // 35
        QStringLiteral(" rowspan=\"9\""),  // 36
        QStringLiteral(" rowspan=\"10\""), // 37
        QStringLiteral(" rowspan=\"11\""), // 38
        QStringLiteral(" rowspan=\"12\""), // 39
        QStringLiteral(" rowspan=\"13\""), // 40
        QStringLiteral(" rowspan=\"14\""), // 41
        QStringLiteral(" rowspan=\"15\""), // 42
        QStringLiteral(" rowspan=\"16\""), // 43
        QStringLiteral(" rowspan=\"17\""), // 44
        QStringLiteral(" rowspan=\"18\""), // 45
        QStringLiteral(" rowspan=\"19\""), // 46
        QStringLiteral(" rowspan=\"20\""), // 47
        QStringLiteral("<td></td>"),       // 48 — 自闭合空单元格
        QStringLiteral("<eos>"),           // 49 — 序列结束（PaddleOCR end_str）
    };
    return v;
}

// colspan token id → span 值（id 10..28 → span 2..20）。
inline int colspanFromId(int id) { return id - 8; }
// rowspan token id → span 值（id 29..47 → span 2..20）。
inline int rowspanFromId(int id) { return id - 27; }

// 单元格起始 token id 集合：<td (7) 和 <td></td> (48)。
inline bool isCellToken(int id) { return id == 7 || id == 48; }

} // namespace

QStringList TableStructureDetector::vocabulary()
{
    return vocab();
}

TableStructureDetector::TableStructureDetector(OrtInferenceEngine *engine)
    : m_engine(engine)
{
}

TableStructureDetector::~TableStructureDetector() = default;

bool TableStructureDetector::available() const
{
    return m_engine && m_engine->isLoaded();
}

bool TableStructureDetector::detect(const QImage &image, QList<DetectedCell> &cells, QString &error)
{
    cells.clear();
    if (!available()) {
        error = QStringLiteral("表格结构检测器不可用：ORT 模型未加载");
        return false;
    }
    if (image.isNull()) {
        error = QStringLiteral("输入图片无效");
        return false;
    }

    constexpr int kInputSize = 488;
    std::vector<float> input = preprocess(image, kInputSize);
    if (input.empty()) {
        error = QStringLiteral("图片预处理失败");
        return false;
    }

    const std::vector<int64_t> shape = {1, 3, kInputSize, kInputSize};
    std::vector<std::vector<int64_t>> outShapes;
    std::vector<std::vector<float>> outputs = m_engine->run(input, shape, &outShapes);
    if (outputs.empty()) {
        error = m_engine->lastError();
        if (error.isEmpty())
            error = QStringLiteral("ORT 推理无输出");
        return false;
    }
    if (outputs.size() < 2) {
        error = QStringLiteral("ORT 输出节点不足（需结构与 bbox 两个）");
        return false;
    }

    // 模型有 2 个输出。SLANet_plus 导出顺序为 [bbox(8), structure(50)]，
    // 但不能硬编码顺序——通过输出最后一维大小判断：V（词表）较大者为结构输出。
    int structIdx = 1;
    int bboxIdx = 0;
    if (outShapes.size() >= 2) {
        const int64_t v0 = outShapes[0].empty() ? 0 : outShapes[0].back();
        const int64_t v1 = outShapes[1].empty() ? 0 : outShapes[1].back();
        if (v0 > v1) {
            structIdx = 0;
            bboxIdx = 1;
        }
    }
    const auto &structOut = outputs[structIdx];
    const auto &bboxOut = outputs[bboxIdx];

    // V 从结构输出的实际形状获取（最后一维）。
    int V = 0;
    size_t T = 0;
    if (outShapes.size() > size_t(structIdx) && outShapes[structIdx].size() >= 2) {
        V = static_cast<int>(outShapes[structIdx].back());
        const int64_t total = static_cast<int64_t>(structOut.size());
        if (V > 0 && total % V == 0)
            T = total / V;
    }
    // 退回按词表大小推断。
    if (V <= 0 || T == 0) {
        V = vocab().size();
        if (V > 0 && structOut.size() % size_t(V) == 0)
            T = structOut.size() / size_t(V);
    }

    // bbox stride 从 bbox 输出形状获取（最后一维，通常 8）。
    int bboxStride = 4;
    if (outShapes.size() > size_t(bboxIdx) && outShapes[bboxIdx].size() >= 2) {
        const int s = static_cast<int>(outShapes[bboxIdx].back());
        if (s > 0)
            bboxStride = s;
    }

    // 判断 bbox 是否按时间步排列（PaddleOCR SLANet 中 bbox 输出与结构序列等长）。
    bool bboxPerTimeStep = false;
    if (T > 0 && bboxStride > 0) {
        const size_t bboxCount = bboxOut.size() / size_t(bboxStride);
        bboxPerTimeStep = (bboxCount == T);
    }

    // 结构 token：按 [T, V] 逐时间步 argmax。
    std::vector<int> structIds;
    if (V > 0 && T > 0) {
        for (size_t t = 0; t < T; ++t) {
            const float *row = structOut.data() + t * V;
            int best = 0;
            float bestVal = row[0];
            for (int v = 1; v < V; ++v) {
                if (row[v] > bestVal) {
                    bestVal = row[v];
                    best = v;
                }
            }
            structIds.push_back(best);
        }
    } else {
        qWarning() << "TableStructureDetector: cannot determine V from output shape, "
                   << "structOut size =" << structOut.size() << "V =" << V;
        error = QStringLiteral("结构输出形状解析失败");
        return false;
    }

    // 遇到 <eos>（id=49）提前截断（SLANet 输出含填充）。
    const int eosId = 49;
    auto eosIt = std::find(structIds.begin(), structIds.end(), eosId);
    if (eosIt != structIds.end())
        structIds.erase(eosIt + 1, structIds.end());

    cells = decodeStructure(structIds, bboxOut, image.size(), bboxStride, bboxPerTimeStep);
    if (cells.isEmpty()) {
        qWarning() << "TableStructureDetector: decodeStructure returned 0 cells."
                   << "structIds count =" << structIds.size()
                   << "bboxOut count =" << bboxOut.size()
                   << "V =" << V << "T =" << T << "bboxStride =" << bboxStride
                   << "bboxPerTimeStep =" << bboxPerTimeStep
                   << "structIdx =" << structIdx << "bboxIdx =" << bboxIdx;
        // 打印前 50 个 token id 辅助排查。
        QString idsStr;
        for (size_t i = 0; i < structIds.size() && i < 50; ++i)
            idsStr += QString::number(structIds[i]) + QLatin1Char(' ');
        qWarning() << "TableStructureDetector: first 50 structIds:" << idsStr;
        error = QStringLiteral("未识别到表格（结构解码无单元格）");
        return false;
    }
    return true;
}

std::vector<float> TableStructureDetector::preprocess(const QImage &image, int inputSize)
{
    if (image.isNull() || inputSize <= 0)
        return {};

    QImage scaled = image.scaled(inputSize, inputSize, Qt::IgnoreAspectRatio,
                                 Qt::SmoothTransformation)
                        .convertToFormat(QImage::Format_RGB888);
    if (scaled.isNull())
        return {};

    constexpr float mean[3] = {0.485f, 0.456f, 0.406f};
    constexpr float std[3] = {0.229f, 0.224f, 0.225f};

    const int bytesPerLine = scaled.bytesPerLine();
    const uchar *bits = scaled.constBits();
    std::vector<float> tensor(static_cast<size_t>(3) * inputSize * inputSize, 0.0f);
    const size_t planeSize = static_cast<size_t>(inputSize) * inputSize;

    for (int y = 0; y < inputSize; ++y) {
        const uchar *line = bits + size_t(y) * bytesPerLine;
        for (int x = 0; x < inputSize; ++x) {
            const int base = x * 3;
            const float r = line[base] / 255.0f;
            const float g = line[base + 1] / 255.0f;
            const float b = line[base + 2] / 255.0f;
            tensor[y * inputSize + x] = (r - mean[0]) / std[0];
            tensor[planeSize + y * inputSize + x] = (g - mean[1]) / std[1];
            tensor[2 * planeSize + y * inputSize + x] = (b - mean[2]) / std[2];
        }
    }
    return tensor;
}

QList<DetectedCell> TableStructureDetector::decodeStructure(const std::vector<int> &structureIds,
                                                            const std::vector<float> &bboxes,
                                                            const QSize &imageSize,
                                                            int bboxStride,
                                                            bool bboxPerTimeStep)
{
    const QStringList &v = vocab();
    const int V = v.size();
    if (bboxStride < 4)
        bboxStride = 4;

    QList<DetectedCell> cells;
    int row = 0;
    int col = 0;
    // spanRows[c] = 该列还被上方 rowspan 占用的行数。
    // 设为 rowSpan（而非 rowSpan-1），因为 advanceRow 会递减一次：
    // 若 rowSpan=2，emitCell 设 spanRows[c]=2，advanceRow 后变为 1（下一行仍被占用），
    // 再 advanceRow 后变为 0（释放）。这保证跨行单元格在所有占用行中都被跳过。
    QMap<int, int> spanRows;
    int cellIndex = 0;       // 单元格计数器（当 bbox 非按时间步排列时使用）
    bool inTd = false;       // 正在累积 <td ...> 属性
    int pendingColSpan = 1;
    int pendingRowSpan = 1;

    auto advanceRow = [&]() {
        // 行结束：递减各列的 rowspan 占用计数。
        for (int c : spanRows.keys()) {
            int left = spanRows[c] - 1;
            if (left <= 0)
                spanRows.remove(c);
            else
                spanRows[c] = left;
        }
        ++row;
        col = 0;
    };

    auto emitCell = [&](int rowSpan, int colSpan, int timeStep) {
        // 跳过被上方 rowspan 覆盖的列。
        while (spanRows.value(col, 0) > 0)
            ++col;
        const int startCol = col;

        DetectedCell cell;
        cell.row = row;
        cell.col = startCol;
        cell.rowSpan = rowSpan;
        cell.colSpan = colSpan;

        // bbox 索引：若按时间步排列则用 timeStep，否则用 cellIndex。
        const int bboxIdxVal = bboxPerTimeStep ? timeStep : cellIndex;
        if (bboxIdxVal >= 0 && size_t(bboxIdxVal + 1) * size_t(bboxStride) <= bboxes.size()) {
            const size_t bi = size_t(bboxIdxVal) * size_t(bboxStride);
            if (bboxStride >= 8) {
                // 4 多边形顶点: (x1,y1),(x2,y2),(x3,y3),(x4,y4)
                float xs[4] = {bboxes[bi], bboxes[bi + 2], bboxes[bi + 4], bboxes[bi + 6]};
                float ys[4] = {bboxes[bi + 1], bboxes[bi + 3], bboxes[bi + 5], bboxes[bi + 7]};
                const float xmin = *std::min_element(xs, xs + 4);
                const float ymin = *std::min_element(ys, ys + 4);
                const float xmax = *std::max_element(xs, xs + 4);
                const float ymax = *std::max_element(ys, ys + 4);
                cell.bbox = QRectF(QPointF(xmin, ymin), QPointF(xmax, ymax));
            } else {
                cell.bbox = QRectF(QPointF(bboxes[bi], bboxes[bi + 1]),
                                   QPointF(bboxes[bi + 2], bboxes[bi + 3]));
            }
        }
        cells.append(cell);

        for (int c = startCol; c < startCol + colSpan; ++c) {
            if (rowSpan > 1)
                spanRows[c] = rowSpan;  // 设置占用行数（含当前行）
            else
                spanRows.remove(c);
        }
        col += colSpan;
        ++cellIndex;
    };

    for (size_t i = 0; i < structureIds.size(); ++i) {
        const int id = structureIds[i];
        if (id < 0 || id >= V)
            continue;
        const QString &tok = v.at(id);

        if (inTd) {
            // 累积属性直到 ">" 闭合。
            if (tok == QStringLiteral(">")) {
                emitCell(pendingRowSpan, pendingColSpan, int(i));
                pendingColSpan = 1;
                pendingRowSpan = 1;
                inTd = false;
            } else if (tok == QStringLiteral("</td>")) {
                // 容错：属性未闭合但遇到 </td>，按已收集属性闭合。
                emitCell(pendingRowSpan, pendingColSpan, int(i));
                pendingColSpan = 1;
                pendingRowSpan = 1;
                inTd = false;
            } else if (id >= 10 && id <= 28) {
                // colspan：span = id - 8（colspanFromId）
                pendingColSpan = colspanFromId(id);
            } else if (id >= 29 && id <= 47) {
                // rowspan：span = id - 27（rowspanFromId）
                pendingRowSpan = rowspanFromId(id);
            }
            continue;
        }

        if (tok == QStringLiteral("<tr>")) {
            // 新行开始。
        } else if (tok == QStringLiteral("</tr>")) {
            advanceRow();
        } else if (tok == QStringLiteral("<td")) {
            inTd = true;
            pendingColSpan = 1;
            pendingRowSpan = 1;
        } else if (tok == QStringLiteral("<td></td>")) {
            // 自闭合空单元格。
            emitCell(1, 1, int(i));
        }
        // 其它 token（<sos>/<eos>/<thead>/</thead>/<tbody>/</tbody>/</td>）忽略。
    }

    // 容错：流结束时仍在累积属性，按已收集属性闭合。
    if (inTd) {
        emitCell(pendingRowSpan, pendingColSpan, int(structureIds.size() - 1));
    }

    // 归一化 bbox（0~1）缩放到原图尺寸。
    for (DetectedCell &cell : cells) {
        if (imageSize.width() > 0 && imageSize.height() > 0) {
            const QRectF nb = cell.bbox;
            cell.bbox = QRectF(QPointF(nb.left() * imageSize.width(),
                                       nb.top() * imageSize.height()),
                               QPointF(nb.right() * imageSize.width(),
                                       nb.bottom() * imageSize.height()));
        }
    }

    return cells;
}

D_TABLERECOGNIZER_END_NAMESPACE
