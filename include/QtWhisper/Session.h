#pragma once
#include <QObject>
#include <QThread>
#include <QString>
#include <QVector>
#include <QSharedPointer>
#include <QtWhisper/IEngine.h>
#include <QtWhisper/Types.h>

namespace QtWhisper {

class Session : public QObject {
    Q_OBJECT

public:
    explicit Session(QObject *parent = nullptr);
    ~Session() override;

    void initialize(IEngine* engine);
    void setConfig(const Config& config);

    void loadModel();
    void unloadModel();
    void reloadModel();
    void clear();

    QString           transcription() const { return mTranscription; }
    bool              isProcessing()  const { return mIsProcessing; }
    QtWhisper::Status status()        const { return mStatus; }
    QString           statusText()    const;

public slots:
    void processAudioWindow(const QVector<float>& samples);
    void resumeInference();
    void stopInference();

signals:
    void transcriptionProgressChanged(float progress);
    void statusChanged(QtWhisper::Status status);
    void isProcessingChanged(bool isProcessing);
    void transcriptionChanged(const QString& fullText);
    void segmentTranscribed(const QString& segment);
    void errorOccurred(QtWhisper::Error error);
    void audioWindowDropped();
    void reloadRequired();

private slots:
    void onSegmentTranscribed(const QString& segment);
    void onIsProcessingChanged(bool isProcessing);
    void onStatusChanged(QtWhisper::Status status);

private:
    IEngine*               mEngine       = nullptr;
    QThread*               mWorkerThread = nullptr;
    QtWhisper::Status      mStatus       = QtWhisper::Status::Idle;
    QSharedPointer<Config> mConfig       = QSharedPointer<Config>::create();
    QString                mTranscription;
    bool                   mIsProcessing = false;
};

} // namespace QtWhisper
