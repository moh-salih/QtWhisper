#include <QCoreApplication>
#include <QDebug>
#include <vector>

#include <QtWhisper/Session.h>
#include <QtWhisper/Engine.h>
#include <QtWhisper/Types.h>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QtWhisper::Session whisperSession;
    QtWhisper::Engine *whisperEngine = new QtWhisper::Engine();

    // initialize() first — spins up the worker thread and takes ownership of
    // the engine. setConfig() after this point will propagate to the thread.
    whisperSession.initialize(whisperEngine);

    QtWhisper::Config config;
    config.modelPath  = "C:/path/to/ggml-tiny.bin";
    config.language   = "en";
    config.useGpu     = true;
    config.threadCount = 4;
    whisperSession.setConfig(config);

    QObject::connect(&whisperSession, &QtWhisper::Session::statusChanged,
                     [&](QtWhisper::Status status) {
        qInfo() << "[STATUS]:" << whisperSession.statusText();

        if (status == QtWhisper::Status::Ready) {
            qInfo() << "Model loaded. Sending audio...";

            // In a real app this comes from QAudioSource.
            // resumeInference() is not needed here — processWindow resets
            // the abort flag automatically before each run.
            std::vector<float> dummyAudio(16000, 0.0f);
            whisperSession.processAudioWindow(dummyAudio);
        }
    });

    QObject::connect(&whisperSession, &QtWhisper::Session::segmentTranscribed,
                     [](const QString &text) {
        qInfo() << "[TRANSCRIPTION]:" << text;
    });

    QObject::connect(&whisperSession, &QtWhisper::Session::errorEncountered,
                     [](const QString &msg) {
        qCritical() << "[ERROR]:" << msg;
        QCoreApplication::quit();
    });

    whisperSession.loadModel();

    return app.exec();
}
