#pragma once
#include <QObject>
#include <QThread>
#include <QString>
#include <QSharedPointer>
#include <QtWhisper/IEngine.h>
#include <QtWhisper/Types.h>

namespace QtWhisper {

class Session : public QObject {
    Q_OBJECT

public:
    explicit Session(QObject *parent = nullptr);
    ~Session() override;

    // Takes ownership of engine. engine must be heap-allocated and must not be
    // parented, moved, or deleted externally after this call. Calling
    // initialize() more than once is a programming error.
    void initialize(IEngine *engine);

    // May be called before or after initialize(). Changes to language,
    // threadCount, etc. take effect on the next loadModel() call — they do
    // not hot-reload a model that is already running.
    void setConfig(const Config &config);

    void loadModel();
    void unloadModel();
    void reloadModel();
    void clear();

    QString           transcription() const { return mTranscription; }
    bool              isProcessing()  const { return mIsProcessing; }
    QtWhisper::Status status()        const { return mStatus; }
    QString           statusText()    const;

public slots:
    // Submits an audio window for inference. If the engine is still processing
    // a previous window the call is a no-op and audioWindowDropped() is emitted.
    void processAudioWindow(const std::vector<float> &samples);

    // Clears the abort flag after stopInference() so subsequent
    // processAudioWindow() calls are accepted again.
    void resumeInference();

    void stopInference();

signals:
    void transcriptionProgressChanged(float progress); // 0.0 – 1.0
    void statusChanged(QtWhisper::Status status);
    void processingBusyChanged(bool isProcessing);
    void transcriptionChanged(const QString &fullText);
    void segmentTranscribed(const QString &segment);
    void errorEncountered(const QString &message);
    void audioWindowDropped();
    void reloadRequired();

private slots:
    void onSegmentTranscribed(const QString &segment);
    void onProcessingBusyChanged(bool isProcessing);
    void onStatusChanged(QtWhisper::Status status);

private:
    IEngine               *mEngine       = nullptr;
    QThread               *mWorkerThread = nullptr;
    QtWhisper::Status      mStatus       = QtWhisper::Status::Idle;
    QSharedPointer<Config> mConfig       = QSharedPointer<Config>::create();
    QString                mTranscription;
    bool                   mIsProcessing = false;
};

} // namespace QtWhisper
