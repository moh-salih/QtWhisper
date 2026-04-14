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
    Q_PROPERTY(QString transcription READ transcription NOTIFY transcriptionChanged)
    Q_PROPERTY(QtWhisper::Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)

public:
    explicit Session(QObject *parent = nullptr);
    ~Session() override;

    void initialize(IEngine* engine);
    void setConfig(const Config& config);
    void loadModel();
    void unloadModel();

    QString transcription() const { return mTranscription; }
    QtWhisper::Status status() const { return mStatus; }
    QString statusText() const;

    Q_INVOKABLE void clear();

public slots:
    void processAudioWindow(const std::vector<float> &samples);
    void startInference();
    void stopInference();

signals:
    void transcriptionChanged(const QString& text);
    void statusChanged();
    void partialResult(const QString& text);
    void errorOccurred(const QString& msg);

private slots:
    void onPartialResult(const QString& text);

private:
    IEngine                   * mEngine       = nullptr;
    QThread                   * mWorkerThread = nullptr;
    QtWhisper::Status           mStatus       = QtWhisper::Status::Idle;
    QSharedPointer<Config>      mConfig       = QSharedPointer<Config>::create();

    QString                     mTranscription;
};

} // namespace QtWhisper
