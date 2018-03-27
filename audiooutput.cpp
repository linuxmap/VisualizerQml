#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QDebug>
#include <qmath.h>
#include <qendian.h>

#include "audiooutput.h"

#define PUSH_MODE_LABEL "Enable push mode"
#define PULL_MODE_LABEL "Enable pull mode"
#define SUSPEND_LABEL   "Suspend playback"
#define RESUME_LABEL    "Resume playback"
#define VOLUME_LABEL    "Volume:"

const int DurationSeconds = 1;
const int ToneSampleRateHz = 600;
const int DataSampleRateHz = 44100;
const int BufferSize = 32768;


Generator::Generator(const QAudioFormat &format,
	qint64 durationUs,
	int sampleRate,
	QObject *parent)
	: QIODevice(parent)
	, m_pos(0)
{
	if (format.isValid())
		generateData(format, durationUs, sampleRate);
}

Generator::~Generator()
{

}

void Generator::start()
{
	open(QIODevice::ReadOnly);
}

void Generator::stop()
{
	m_pos = 0;
	close();
}

void Generator::generateData(const QAudioFormat &format, qint64 durationUs, int sampleRate)
{
	const int channelBytes = format.sampleSize() / 8;
	const int sampleBytes = format.channelCount() * channelBytes;

	qint64 length = (format.sampleRate() * format.channelCount() * (format.sampleSize() / 8))
		* durationUs / 100000;

	Q_ASSERT(length % sampleBytes == 0);
	Q_UNUSED(sampleBytes) // suppress warning in release builds

		m_buffer.resize(length);
	unsigned char *ptr = reinterpret_cast<unsigned char *>(m_buffer.data());
	int sampleIndex = 0;

	while (length) {
		const qreal x = qSin(2 * M_PI * sampleRate * qreal(sampleIndex % format.sampleRate()) / format.sampleRate());
		for (int i = 0; i < format.channelCount(); ++i) {
			if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::UnSignedInt) {
				const quint8 value = static_cast<quint8>((1.0 + x) / 2 * 255);
				*reinterpret_cast<quint8*>(ptr) = value;
			}
			else if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::SignedInt) {
				const qint8 value = static_cast<qint8>(x * 127);
				*reinterpret_cast<quint8*>(ptr) = value;
			}
			else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::UnSignedInt) {
				quint16 value = static_cast<quint16>((1.0 + x) / 2 * 65535);
				if (format.byteOrder() == QAudioFormat::LittleEndian)
					qToLittleEndian<quint16>(value, ptr);
				else
					qToBigEndian<quint16>(value, ptr);
			}
			else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::SignedInt) {
				qint16 value = static_cast<qint16>(x * 32767);
				if (format.byteOrder() == QAudioFormat::LittleEndian)
					qToLittleEndian<qint16>(value, ptr);
				else
					qToBigEndian<qint16>(value, ptr);
			}

			ptr += channelBytes;
			length -= channelBytes;
		}
		++sampleIndex;
	}
}

qint64 Generator::readData(char *data, qint64 len)
{
	qint64 total = 0;
	if (!m_buffer.isEmpty()) {
		while (len - total > 0) {
			const qint64 chunk = qMin((m_buffer.size() - m_pos), len - total);
			memcpy(data + total, m_buffer.constData() + m_pos, chunk);
			m_pos = (m_pos + chunk) % m_buffer.size();
			total += chunk;
		}
	}
	return total;
}

qint64 Generator::writeData(const char *data, qint64 len)
{
	Q_UNUSED(data);
	Q_UNUSED(len);

	return 0;
}

qint64 Generator::bytesAvailable() const
{
	return m_buffer.size() + QIODevice::bytesAvailable();
}

AudioOutput::AudioOutput(QObject *parent) : QObject(parent)
, m_pushTimer(new QTimer(this))
, m_device(QAudioDeviceInfo::defaultOutputDevice())
, m_generator(0)
, m_sampleRate(44100)
, m_channelCnt(1)
, m_sampleSize(16)
, m_codec("audio/pcm")
, m_byteOrder(QAudioFormat::LittleEndian)
, m_sampleType(QAudioFormat::SignedInt)
, m_audioOutput(0)
, m_output(NULL)
//, m_buffer(32768, 0)
{
	for (int i = 0; i < AUDIO_OUTPUT_CHANNEL_MAX; i++)
	{
		m_buffer[i].clear();
	}
	//initializeAudio();
}

void AudioOutput::initializeAudio()
{
	connect(m_pushTimer, SIGNAL(timeout()), this,SLOT(pushTimerExpired()));

	m_format.setSampleRate(m_sampleRate);
	m_format.setChannelCount(m_channelCnt);
	m_format.setSampleSize(m_sampleSize);
	m_format.setCodec(m_codec);
	m_format.setByteOrder(m_byteOrder);
	m_format.setSampleType(m_sampleType);

	qDebug() << "Audio output :"
		<< "Sample rate:" << m_sampleRate
		<< "Channel:" << m_channelCnt
		<<"Sample Size"<<m_sampleSize
		<<"Codec:"<<m_codec
		<<"Sample type:"<<m_sampleType;

	QAudioDeviceInfo info(m_device);
	if (!info.isFormatSupported(m_format)) {
		qWarning() << "Default format not supported - trying to use nearest";
		m_format = info.nearestFormat(m_format);
	}

#if 0
	if (m_generator)
		delete m_generator;
	m_generator = new Generator(m_format, DurationSeconds * 1000000, ToneSampleRateHz, this);
#endif

	createAudioOutput();
}

qint64 AudioOutput::writeData(const int channel,const char * buf, const qint64 len)
{
	if (buf&&len > 0)
	{
		m_bufferLock.lock();
		m_buffer[channel].append(buf, len);
		m_bufferLock.unlock();
		return len;
#if 0

#endif
		
	}
	return 0;
}

void AudioOutput::createAudioOutput()
{
	if (m_audioOutput)
	{
		delete m_audioOutput;
		m_audioOutput = 0;
	}
	m_audioOutput = new QAudioOutput(m_device, m_format, this);
	//m_generator->start();
	//m_audioOutput->start(m_generator);
	//m_audioOutput->setBufferSize(100000000);
	m_output=m_audioOutput->start();
	m_pushTimer->start(20);
	qreal initialVolume = QAudio::convertVolume(m_audioOutput->volume(),
		QAudio::LinearVolumeScale,
		QAudio::LogarithmicVolumeScale);
}

AudioOutput::~AudioOutput()
{

}

void AudioOutput::setSampleRate(const int rate)
{
	m_sampleRate = rate;
}

void AudioOutput::setChannelCount(const int cnt)
{
	m_channelCnt = cnt;
}

void AudioOutput::setSampleSize(const int ss)
{
	m_sampleSize = ss;
}

void AudioOutput::setCodec(const QString & codec)
{
	m_codec = codec;
}

void AudioOutput::setByteOrder(QAudioFormat::Endian byteOrder)
{
	m_byteOrder = byteOrder;
}

void AudioOutput::setSampleType(QAudioFormat::SampleType sampleType)
{
	m_sampleType = sampleType;
}

void AudioOutput::deviceChanged(int index)
{
	m_pushTimer->stop();
	m_generator->stop();
	m_audioOutput->stop();
	m_audioOutput->disconnect(this);
	initializeAudio();
}

void AudioOutput::volumeChanged(int value)
{
	if (m_audioOutput) {
		qreal linearVolume = QAudio::convertVolume(value / qreal(100),
			QAudio::LogarithmicVolumeScale,
			QAudio::LinearVolumeScale);

		m_audioOutput->setVolume(linearVolume);
	}
}

void AudioOutput::pushTimerExpired()
{
	qDebug() << "Timer";
	m_bufferLock.lock();

	if (m_audioOutput && m_audioOutput->state() != QAudio::StoppedState) 
	{
		for (int i = 0; i < AUDIO_OUTPUT_CHANNEL_MAX; i++)
		{
			if (m_buffer[i].isEmpty())continue;
			m_output->setCurrentWriteChannel(i);
			int buffFree = m_audioOutput->bytesFree();
			int periodSize = m_audioOutput->periodSize();
			int chunks = buffFree / periodSize;
			//if (chunks <= 0)qDebug("Audio buffer is overflow");
			//int writedLen = 0;
			int lenToWrite = 0;
			while (chunks > 0)
			{
				lenToWrite = qMin(m_buffer[i].length(), periodSize);
				m_output->write(m_buffer[i].data(), lenToWrite);
				m_buffer->remove(0,lenToWrite);
				//writedLen += lenToWrite;
				if (m_buffer->isEmpty())
					break;
				chunks--;
			}
		}
	}
	m_bufferLock.unlock();

}


void AudioOutput::toggleSuspendResume()
{
	if (m_audioOutput->state() == QAudio::SuspendedState) {
		m_audioOutput->resume();
	}
	else if (m_audioOutput->state() == QAudio::ActiveState) {
		m_audioOutput->suspend();
	}
	else if (m_audioOutput->state() == QAudio::StoppedState) {
		m_audioOutput->resume();
	}
	else if (m_audioOutput->state() == QAudio::IdleState) {
		// no-op
	}
}

