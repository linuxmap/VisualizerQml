
#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <math.h>

#include <QAudioOutput>
#include <QByteArray>
#include <QIODevice>
#include <QObject>
#include <QTimer>
#include <QMutex>

#define AUDIO_OUTPUT_CHANNEL_MAX   8

class Generator : public QIODevice
{
	Q_OBJECT

public:
	Generator(const QAudioFormat &format, qint64 durationUs, int sampleRate, QObject *parent);
	~Generator();

	void start();
	void stop();

	qint64 readData(char *data, qint64 maxlen) override;
	qint64 writeData(const char *data, qint64 len) override;
	qint64 bytesAvailable() const override;

private:
	void generateData(const QAudioFormat &format, qint64 durationUs, int sampleRate);

private:
	qint64 m_pos;
	QByteArray m_buffer;
};

class AudioOutput : public QObject
{
    Q_OBJECT

public:
    AudioOutput(QObject *parent=nullptr);
    ~AudioOutput();
	void setSampleRate(const int rate);
	void setChannelCount(const int cnt);
	void setSampleSize(const int ss);
	void setCodec(const QString &codec);
	void setByteOrder(QAudioFormat::Endian byteOrder);
	void setSampleType(QAudioFormat::SampleType sampleType);
	void initializeAudio();
	qint64 writeData(const int channel,const char *buf, const qint64 len);
private:
    void createAudioOutput();

private:
    QTimer *m_pushTimer;

    QAudioDeviceInfo m_device;
    Generator *m_generator;
    QAudioOutput *m_audioOutput;
    QIODevice *m_output; // not owned
    QAudioFormat m_format;
	int m_sampleRate;
	int m_channelCnt;
	int m_sampleSize;
	QString m_codec;
	QAudioFormat::Endian m_byteOrder;
	QAudioFormat::SampleType m_sampleType;
    QByteArray m_buffer[AUDIO_OUTPUT_CHANNEL_MAX];
	QMutex m_bufferLock;

private slots:
    void pushTimerExpired();
    void toggleSuspendResume();
    void deviceChanged(int index);
    void volumeChanged(int);
};

#endif // AUDIOOUTPUT_H
