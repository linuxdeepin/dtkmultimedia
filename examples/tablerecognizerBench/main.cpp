// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <dtablerecognizer.h>

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTextStream>
#include <algorithm>
#include <cmath>
#include <thread>
#include <vector>

D_TABLERECOGNIZER_USE_NAMESPACE

namespace {

// 单张图片的计时记录。
struct TimingRecord
{
    QString fileName;
    bool success = false;
    QString source;
    qint64 structureMs = 0;
    qint64 ocrMs = 0;
    qint64 totalMs = 0;
};

// 计算统计量：均值、中位数、P90、最小、最大。
struct Stats
{
    double mean = 0;
    double median = 0;
    double p90 = 0;
    qint64 min = 0;
    qint64 max = 0;
};

// 对一组耗时值计算统计量（跳过失败记录）。
Stats computeStats(std::vector<qint64> values)
{
    Stats s;
    if (values.empty())
        return s;

    std::sort(values.begin(), values.end());
    const size_t n = values.size();
    s.min = values.front();
    s.max = values.back();

    double sum = 0;
    for (auto v : values)
        sum += static_cast<double>(v);
    s.mean = sum / static_cast<double>(n);

    // 中位数
    if (n % 2 == 0)
        s.median = (static_cast<double>(values[n / 2 - 1]) + static_cast<double>(values[n / 2])) / 2.0;
    else
        s.median = static_cast<double>(values[n / 2]);

    // P90：使用最近秩方法（nearest rank）
    const size_t p90Index = static_cast<size_t>(std::ceil(0.9 * n)) - 1;
    s.p90 = static_cast<double>(values[std::min(p90Index, n - 1)]);

    return s;
}

// 同步识别单张图片，阻塞等待 recognitionDone 信号。
TimingRecord recognizeImage(DTableRecognizer &recognizer, const QImage &image,
                            const QString &fileName)
{
    TimingRecord rec;
    rec.fileName = fileName;

    bool gotResult = false;
    QObject::connect(&recognizer, &DTableRecognizer::recognitionDone,
                     [&rec, &gotResult](const DTableResult &result) {
                         rec.success = result.success;
                         rec.source = result.source;
                         rec.structureMs = result.structureMs;
                         rec.ocrMs = result.ocrMs;
                         rec.totalMs = result.totalMs;
                         gotResult = true;
                     },
                     Qt::QueuedConnection);

    recognizer.recognizeAsync(image);

    // 阻塞等待信号回投。
    while (!gotResult)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    return rec;
}

// 输出 JSON 格式结果。
void outputJson(const std::vector<TimingRecord> &records,
                const Stats &structStats, const Stats &ocrStats, const Stats &totalStats,
                int threads, int imageCount, QTextStream &out)
{
    QJsonObject root;
    root["threads"] = threads;
    root["imageCount"] = imageCount;
    root["successCount"] = static_cast<int>(std::count_if(records.begin(), records.end(),
                                                          [](const TimingRecord &r) { return r.success; }));

    auto statsToJson = [](const Stats &s) {
        QJsonObject obj;
        obj["mean"] = QJsonValue(s.mean);
        obj["median"] = QJsonValue(s.median);
        obj["p90"] = QJsonValue(s.p90);
        obj["min"] = QJsonValue(static_cast<double>(s.min));
        obj["max"] = QJsonValue(static_cast<double>(s.max));
        return obj;
    };

    QJsonObject summary;
    summary["structureMs"] = statsToJson(structStats);
    summary["ocrMs"] = statsToJson(ocrStats);
    summary["totalMs"] = statsToJson(totalStats);
    root["summary"] = summary;

    QJsonArray arr;
    for (const auto &r : records) {
        QJsonObject obj;
        obj["file"] = r.fileName;
        obj["success"] = r.success;
        obj["source"] = r.source;
        obj["structureMs"] = QJsonValue(static_cast<double>(r.structureMs));
        obj["ocrMs"] = QJsonValue(static_cast<double>(r.ocrMs));
        obj["totalMs"] = QJsonValue(static_cast<double>(r.totalMs));
        arr.append(obj);
    }
    root["results"] = arr;

    out << QJsonDocument(root).toJson(QJsonDocument::Indented);
}

// 输出 CSV 格式结果。
void outputCsv(const std::vector<TimingRecord> &records,
               const Stats &structStats, const Stats &ocrStats, const Stats &totalStats,
               QTextStream &out)
{
    out << "file,success,source,structureMs,ocrMs,totalMs\n";
    for (const auto &r : records) {
        out << r.fileName << ","
            << (r.success ? "true" : "false") << ","
            << r.source << ","
            << r.structureMs << ","
            << r.ocrMs << ","
            << r.totalMs << "\n";
    }

    out << "\n";
    out << "summary,metric,mean,median,p90,min,max\n";
    out << "summary,structureMs," << structStats.mean << "," << structStats.median << ","
        << structStats.p90 << "," << structStats.min << "," << structStats.max << "\n";
    out << "summary,ocrMs," << ocrStats.mean << "," << ocrStats.median << ","
        << ocrStats.p90 << "," << ocrStats.min << "," << ocrStats.max << "\n";
    out << "summary,totalMs," << totalStats.mean << "," << totalStats.median << ","
        << totalStats.p90 << "," << totalStats.min << "," << totalStats.max << "\n";
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("tablerecognizerBench"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("libdtk6tablerecognizer 三平台速度测试 CLI\n"
                       "输入图片目录，输出每张图的结构检测/OCR/总识别耗时及汇总统计。"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption threadsOpt(QStringList() << "threads",
        QStringLiteral("ORT 推理线程数（默认取物理核数）"), QStringLiteral("N"));
    QCommandLineOption warmupOpt(QStringList() << "warmup",
        QStringLiteral("预热迭代次数，默认 3（预热结果不计入统计）"), QStringLiteral("N"), QStringLiteral("3"));
    QCommandLineOption formatOpt(QStringList() << "format",
        QStringLiteral("输出格式：json 或 csv，默认 json"), QStringLiteral("json|csv"), QStringLiteral("json"));
    QCommandLineOption outputOpt(QStringList() << "output" << "o",
        QStringLiteral("输出到文件（默认 stdout）"), QStringLiteral("FILE"));

    parser.addOption(threadsOpt);
    parser.addOption(warmupOpt);
    parser.addOption(formatOpt);
    parser.addOption(outputOpt);
    parser.addPositionalArgument(QStringLiteral("dir"),
        QStringLiteral("图片目录（支持 png/jpg/jpeg/bmp/tiff/webp）"));

    parser.process(app);

    const QStringList posArgs = parser.positionalArguments();
    if (posArgs.isEmpty()) {
        qCritical("Error: missing image directory argument.");
        parser.showHelp(1);
    }

    const QString dirPath = posArgs.first();
    const QDir dir(dirPath);
    if (!dir.exists()) {
        qCritical("Error: directory does not exist: %s", qPrintable(dirPath));
        return 1;
    }

    // 线程数：优先 --threads，否则默认物理核数（由库内 OrtInferenceEngine 读取环境变量）。
    int threads = static_cast<int>(std::thread::hardware_concurrency());
    if (threads < 1)
        threads = 1;
    if (parser.isSet(threadsOpt)) {
        bool ok = false;
        const int val = parser.value(threadsOpt).toInt(&ok);
        if (ok && val > 0) {
            threads = val;
        } else {
            qCritical("Error: invalid --threads value: %s", qPrintable(parser.value(threadsOpt)));
            return 1;
        }
    }
    // 设置环境变量，OrtInferenceEngine 加载模型时读取。
    qputenv("TABLEREC_ORT_THREADS", QByteArray::number(threads));

    const int warmup = parser.value(warmupOpt).toInt();
    const QString format = parser.value(formatOpt).toLower();
    const QString outputFile = parser.value(outputOpt);

    if (format != "json" && format != "csv") {
        qCritical("Error: --format must be json or csv");
        return 1;
    }

    // 收集目录下的图片文件。
    const QStringList filters = {"*.png", "*.jpg", "*.jpeg", "*.bmp", "*.tiff", "*.webp"};
    const QFileInfoList imageFiles = dir.entryInfoList(filters, QDir::Files, QDir::Name);
    if (imageFiles.isEmpty()) {
        qCritical("Error: no image files found in: %s", qPrintable(dirPath));
        return 1;
    }

    qInfo("tablerecognizerBench: %d images, threads=%d, warmup=%d, format=%s",
          static_cast<int>(imageFiles.size()), threads, warmup, qPrintable(format));

    DTableRecognizer recognizer;

    // 预热：用第一张图跑 N 次预热推理，排除冷启动开销。
    if (warmup > 0 && !imageFiles.isEmpty()) {
        const QImage warmupImg(imageFiles.first().absoluteFilePath());
        if (!warmupImg.isNull()) {
            qInfo("Warmup: %d iterations on %s", warmup, qPrintable(imageFiles.first().fileName()));
            for (int i = 0; i < warmup; ++i)
                recognizeImage(recognizer, warmupImg, imageFiles.first().fileName());
        }
    }

    // 正式测试：逐张识别，记录计时。
    std::vector<TimingRecord> records;
    records.reserve(imageFiles.size());

    for (const QFileInfo &fileInfo : imageFiles) {
        const QImage image(fileInfo.absoluteFilePath());
        if (image.isNull()) {
            qWarning("Failed to load image: %s", qPrintable(fileInfo.fileName()));
            TimingRecord rec;
            rec.fileName = fileInfo.fileName();
            rec.success = false;
            records.push_back(rec);
            continue;
        }

        const TimingRecord rec = recognizeImage(recognizer, image, fileInfo.fileName());
        qInfo("[%s] %s: struct=%lldms ocr=%lldms total=%lldms %s",
              rec.success ? "OK" : "FAIL",
              qPrintable(rec.fileName),
              rec.structureMs, rec.ocrMs, rec.totalMs,
              qPrintable(rec.source));
        records.push_back(rec);
    }

    // 计算汇总统计（仅统计成功的记录）。
    std::vector<qint64> structTimes, ocrTimes, totalTimes;
    for (const auto &r : records) {
        if (r.success) {
            structTimes.push_back(r.structureMs);
            ocrTimes.push_back(r.ocrMs);
            totalTimes.push_back(r.totalMs);
        }
    }

    const Stats structStats = computeStats(std::move(structTimes));
    const Stats ocrStats = computeStats(std::move(ocrTimes));
    const Stats totalStats = computeStats(std::move(totalTimes));

    qInfo("\n=== Summary (threads=%d, images=%d, success=%zu) ===",
          threads, static_cast<int>(records.size()),
          std::count_if(records.begin(), records.end(), [](const TimingRecord &r) { return r.success; }));
    qInfo("structureMs: mean=%.1f median=%.1f p90=%.1f min=%lld max=%lld",
          structStats.mean, structStats.median, structStats.p90,
          structStats.min, structStats.max);
    qInfo("ocrMs:       mean=%.1f median=%.1f p90=%.1f min=%lld max=%lld",
          ocrStats.mean, ocrStats.median, ocrStats.p90,
          ocrStats.min, ocrStats.max);
    qInfo("totalMs:     mean=%.1f median=%.1f p90=%.1f min=%lld max=%lld",
          totalStats.mean, totalStats.median, totalStats.p90,
          totalStats.min, totalStats.max);

    // 输出结构化结果。
    QString outputText;
    QTextStream outStream(&outputText);
    if (format == "json")
        outputJson(records, structStats, ocrStats, totalStats,
                   threads, static_cast<int>(records.size()), outStream);
    else
        outputCsv(records, structStats, ocrStats, totalStats, outStream);
    outStream.flush();

    if (outputFile.isEmpty()) {
        QTextStream(stdout) << outputText;
    } else {
        QFile f(outputFile);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qCritical("Error: cannot open output file: %s", qPrintable(outputFile));
            return 1;
        }
        f.write(outputText.toUtf8());
        f.close();
    }

    return 0;
}
