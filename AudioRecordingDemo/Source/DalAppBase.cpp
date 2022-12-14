//
// Copyright © 2021-2022 Audinate Pty Ltd ACN 120 828 006 (Audinate). All rights reserved.
//
//
// 1.	Subject to the terms and conditions of this Licence, Audinate hereby grants you a worldwide, non-exclusive,
//		no-charge, royalty free licence to copy, modify, merge, publish, redistribute, sublicense, and/or sell the
//		Software, provided always that the following conditions are met:
//		1.1.	the Software must accompany, or be incorporated in a licensed Audinate product, solution or offering
//				or be used in a product, solution or offering which requires the use of another licensed Audinate
//				product, solution or offering. The Software is not for use as a standalone product without any
//				reference to Audinate's products;
//		1.2.	the Software is provided as part of example code and as guidance material only without any warranty
//				or expectation of performance, compatibility, support, updates or security; and
//		1.3.	the above copyright notice and this License must be included in all copies or substantial portions
//				of the Software, and all derivative works of the Software, unless the copies or derivative works are
//				solely in the form of machine-executable object code generated by the source language processor.
//
// 2.	TO THE EXTENT PERMITTED BY APPLICABLE LAW, THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//		EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
//		PURPOSE AND NONINFRINGEMENT.
//
// 3.	TO THE FULLEST EXTENT PERMITTED BY APPLICABLE LAW, IN NO EVENT SHALL AUDINATE BE LIABLE ON ANY LEGAL THEORY
//		(INCLUDING, WITHOUT LIMITATION, IN AN ACTION FOR BREACH OF CONTRACT, NEGLIGENCE OR OTHERWISE) FOR ANY CLAIM,
//		LOSS, DAMAGES OR OTHER LIABILITY HOWSOEVER INCURRED.  WITHOUT LIMITING THE SCOPE OF THE PREVIOUS SENTENCE THE
//		EXCLUSION OF LIABILITY SHALL INCLUDE: LOSS OF PRODUCTION OR OPERATION TIME, LOSS, DAMAGE OR CORRUPTION OF
//		DATA OR RECORDS; OR LOSS OF ANTICIPATED SAVINGS, OPPORTUNITY, REVENUE, PROFIT OR GOODWILL, OR OTHER ECONOMIC
//		LOSS; OR ANY SPECIAL, INCIDENTAL, INDIRECT, CONSEQUENTIAL, PUNITIVE OR EXEMPLARY DAMAGES, ARISING OUT OF OR
//		IN CONNECTION WITH THIS AGREEMENT, ACCESS OF THE SOFTWARE OR ANY OTHER DEALINGS WITH THE SOFTWARE, EVEN IF
//		AUDINATE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH CLAIM, LOSS, DAMAGES OR OTHER LIABILITY.
//
// 4.	APPLICABLE LEGISLATION SUCH AS THE AUSTRALIAN CONSUMER LAW MAY APPLY REPRESENTATIONS, WARRANTIES, OR CONDITIONS,
//		OR IMPOSES OBLIGATIONS OR LIABILITY ON AUDINATE THAT CANNOT BE EXCLUDED, RESTRICTED OR MODIFIED TO THE FULL
//		EXTENT SET OUT IN THE EXPRESS TERMS OF THIS CLAUSE ABOVE "CONSUMER GUARANTEES".	 TO THE EXTENT THAT SUCH CONSUMER
//		GUARANTEES CONTINUE TO APPLY, THEN TO THE FULL EXTENT PERMITTED BY THE APPLICABLE LEGISLATION, THE LIABILITY OF
//		AUDINATE UNDER THE RELEVANT CONSUMER GUARANTEE IS LIMITED (WHERE PERMITTED AT AUDINATE'S OPTION) TO ONE OF
//		FOLLOWING REMEDIES OR SUBSTANTIALLY EQUIVALENT REMEDIES:
//		4.1.	THE REPLACEMENT OF THE SOFTWARE, THE SUPPLY OF EQUIVALENT SOFTWARE, OR SUPPLYING RELEVANT SERVICES AGAIN;
//		4.2.	THE REPAIR OF THE SOFTWARE;
//		4.3.	THE PAYMENT OF THE COST OF REPLACING THE SOFTWARE, OF ACQUIRING EQUIVALENT SOFTWARE, HAVING THE RELEVANT
//				SERVICES SUPPLIED AGAIN, OR HAVING THE SOFTWARE REPAIRED.
//
// 5.	This License does not grant any permissions or rights to use the trade marks (whether registered or unregistered),
//		the trade names, or product names of Audinate.
//
// 6.	If you choose to redistribute or sell the Software you may elect to offer support, maintenance, warranties,
//		indemnities or other liability obligations or rights consistent with this License. However, you may only act on
//		your own behalf and must not bind Audinate. You agree to indemnify and hold harmless Audinate, and its affiliates
//		form any liability claimed or incurred by reason of your offering or accepting any additional warranty or additional
//		liability.
//
//  DalAppBase.cpp
//  DAL example common DAL application code.
#include "DalAppBase.hpp"

namespace DAL {
	static bool g_running = false;
	static bool g_restart = false;
	static std::ofstream mLog;
	static std::uint16_t  DAL_EXAMPLE_ARCP_PORT = 30440;
	static std::uint16_t  DAL_EXAMPLE_ARCP_LOCAL_PORT = 30441;
	static std::uint16_t  DAL_EXAMPLE_DBCP_PORT = 30455;
	static std::uint16_t  DAL_EXAMPLE_AUDIO_BASE_PORT = 30336;
	static std::uint16_t  DAL_EXAMPLE_CONMON_CHANNEL_PORT = 34700;
	static std::uint16_t  DAL_EXAMPLE_CMCP_PORT = 34800;
	static std::uint16_t  DAL_EXAMPLE_CONMON_CLIENT_PORT = 34900;
	static std::uint16_t  DAL_EXAMPLE_WEB_SOCKET_PORT = 0; // Request ephemeral port

	// On Windows the Domain Client socket uses a port number, on macOS it uses a UNIX socket path
	// MDNS Client is only required for Windows
#ifdef _WIN32
	static std::uint16_t DAL_EXAMPLE_DOMAIN_CLIENT_SOCKET_DESCRIPTOR = 34001;
	static std::uint16_t DAL_EXAMPLE_MDNS_CLIENT_PORT = 34002;
#else
	static std::string DAL_EXAMPLE_DOMAIN_CLIENT_SOCKET_DESCRIPTOR "/tmp/DalExamples"
#endif

		// Defaut config values.
#define DEFAULT_SAMPLE_RATE 48000
#define DEFAULT_SAMPLES_PER_PERIOD 128
#define DEFAULT_PERIODS_PER_BUFFER 64
#define DEFAULT_ENCODING 24
#define DEFAULT_TX_CHANS 2
#define DEFAULT_RX_CHANS 2
#define DEFAULT_ACTIVATION_DIRECTORY "."
#define DEFAULT_TIME_SOURCE Audinate::DAL::TimeSource::Ptp
#define DEFAULT_PROCESS_PATH "."
#define DEFAULT_LOG_LEVEL Audinate::DAL::LogLevel::Warning

		DalConfig::DalConfig() {
		setEncoding(DEFAULT_ENCODING);
		setNumRxChannels(DEFAULT_TX_CHANS);
		setNumTxChannels(DEFAULT_RX_CHANS);
		setTimeSource(DEFAULT_TIME_SOURCE);
		setActivationDirectory(DEFAULT_ACTIVATION_DIRECTORY);
		setSamplerate(DEFAULT_SAMPLE_RATE);
		setSamplesPerPeriod(DEFAULT_SAMPLES_PER_PERIOD);
		setPeriodsPerBuffer(DEFAULT_PERIODS_PER_BUFFER);
		setProtocolSocketDescriptor(Audinate::DAL::Protocol::Arcp, Audinate::DAL::SocketDescriptor(DAL_EXAMPLE_ARCP_PORT));
		setProtocolSocketDescriptor(Audinate::DAL::Protocol::ArcpLocal, Audinate::DAL::SocketDescriptor(DAL_EXAMPLE_ARCP_LOCAL_PORT));
		setProtocolSocketDescriptor(Audinate::DAL::Protocol::Dbcp, Audinate::DAL::SocketDescriptor(DAL_EXAMPLE_DBCP_PORT));
		setProtocolSocketDescriptor(Audinate::DAL::Protocol::AudioBase, Audinate::DAL::SocketDescriptor(DAL_EXAMPLE_AUDIO_BASE_PORT));
		setProtocolSocketDescriptor(Audinate::DAL::Protocol::ConmonChannels, Audinate::DAL::SocketDescriptor(DAL_EXAMPLE_CONMON_CHANNEL_PORT));
		setProtocolSocketDescriptor(Audinate::DAL::Protocol::Cmcp, Audinate::DAL::SocketDescriptor(DAL_EXAMPLE_CMCP_PORT));
		setProtocolSocketDescriptor(Audinate::DAL::Protocol::ConmonClient, Audinate::DAL::SocketDescriptor(DAL_EXAMPLE_CONMON_CLIENT_PORT));
		setProtocolSocketDescriptor(Audinate::DAL::Protocol::DomainClientProxy, Audinate::DAL::SocketDescriptor(DAL_EXAMPLE_DOMAIN_CLIENT_SOCKET_DESCRIPTOR));
		setProtocolSocketDescriptor(Audinate::DAL::Protocol::WebSocket, Audinate::DAL::SocketDescriptor(DAL_EXAMPLE_WEB_SOCKET_PORT));
#ifdef _WIN32
		setProtocolSocketDescriptor(Audinate::DAL::Protocol::MdnsClient, Audinate::DAL::SocketDescriptor(DAL_EXAMPLE_MDNS_CLIENT_PORT));
#endif
		setLogLevel(DEFAULT_LOG_LEVEL);
		setProcessPath(DEFAULT_PROCESS_PATH);
		setLoggingPath(DEFAULT_PROCESS_PATH);
	}

	bool DalAppBase::isSupportedSampleRate(uint32_t sampleRate) const
	{
		std::vector<uint32_t> supportedSampleRates = { 48000 };
		auto findResult =
			std::find
			(
				supportedSampleRates.begin(),
				supportedSampleRates.end(),
				sampleRate
			);
		return (findResult != supportedSampleRates.end());
	}
	static bool componentApecRunning = false;
	static void handleEvent(Audinate::DAL::Instance& instance, const Audinate::DAL::InstanceEvent& ev)
	{
		switch (ev.getType())
		{
		case Audinate::DAL::InstanceEvent::Type::InstanceStateChanged:
			mLog << "Instance state changed, now " << Audinate::DAL::toString(instance.getInstanceState()) << std::endl << std::flush;
			break;
		case Audinate::DAL::InstanceEvent::Type::ComponentStatusChanged:
			mLog << "Component " << Audinate::DAL::toString(ev.getComponent()) << " status changed, now " << Audinate::DAL::toString(instance.getComponentStatus(ev.getComponent())) << std::endl << std::flush;
			if (ev.getComponent() == Audinate::DAL::Component::Apec)
			{
				componentApecRunning = (instance.getComponentStatus(ev.getComponent()) == Audinate::DAL::ComponentStatus::Running);
			}
			break;
		case Audinate::DAL::InstanceEvent::Type::DomainInfoChanged:
			if (instance.getDomainInfo().mIsEnrolled)
			{
				mLog << "Domain info changed, enrolled in managed domain=" << instance.getDomainInfo().mDomainName << std::endl << std::flush;
			}
			else
			{
				mLog << "Domain info changed, not enrolled" << std::endl << std::flush;
			}
			break;
		case Audinate::DAL::InstanceEvent::Type::DeviceActivationStatusChanged:
			//The activation status of the device has changed. The transfer callback
			//can continue with the current number of channels, but audio will be
			//available only on activated number of channels.
			//To re-configure the DAL application with updated channel counts from
			//DAL::AudioProperties, the DAL instance needs to be restarted.
			g_restart = true;

			if (instance.isDeviceActivated())
			{
				mLog << "Device activation status changed, activated" << std::endl << std::flush;
			}
			else
			{
				mLog << "Device activation status changed, not activated" << std::endl << std::flush;
			}
		}

	}

	static void handleMonitoringEvent(const Audinate::DAL::MonitoringEvent& ev)
	{
		auto ts = ev.getTimestamp();
		mLog << ts.mSeconds << "." << ts.mNanoseconds << ":";
		if (ev.hasType(Audinate::DAL::MonitoringEvent::Type::MaxControlThreadInterval))
		{
			mLog << " control=" << ev.getMaxControlThreadIntervalUs();
		}
		if (ev.hasType(Audinate::DAL::MonitoringEvent::Type::MaxAudioThreadInterval))
		{
			mLog << " audio=" << ev.getMaxAudioThreadIntervalUs();
		}
		if (ev.hasType(Audinate::DAL::MonitoringEvent::Type::LatePacketCount))
		{
			mLog << " late=" << ev.getLatePacketCount();
		}
		if (ev.hasType(Audinate::DAL::MonitoringEvent::Type::NonSequentialPacketCount))
		{
			mLog << " nonSeq=" << ev.getNonSequentialPacketCount();
		}
		mLog << std::endl << std::flush;
	}

	static std::string toString(const Audinate::DAL::Id64& id64)
	{
		std::stringstream ss;
		for (size_t i = 0; i < AUDINATE_DAL_ID64_LENGTH; i++)
		{
			ss << std::hex << (unsigned int)(id64.mData[i]);
		}
		return ss.str();
	}


	int DalAppBase::init(const unsigned char* access_token, DalConfig instanceConfig, bool monitor)
	{
		componentApecRunning = false;
		mLog.open("dal.log");
		// Create DAL
		try
		{
			mDal = Audinate::DAL::createDAL((char*)access_token);
		}
		catch (const Audinate::DAL::DalException& exception)
		{
			mLog << exception.getErrorDescription() << "\t(" << exception.getErrorName() << ")" << std::endl << std::flush;
			g_running = false;
			mDal = nullptr;
			return -1;
		}

		mConfig = instanceConfig;

		Audinate::DAL::Id64 manufacturerId = mDal->getManufacturerId();
		mLog << "ManufacturerId is 0x" << DAL::toString(manufacturerId) << std::endl << std::flush;

		Audinate::DAL::DALVersion dalVersion = Audinate::DAL::getVersion();
		mLog << "DAL version is " << unsigned(dalVersion.mMajor) << "." << unsigned(dalVersion.mMinor) << "." << dalVersion.mBugfix << "." << dalVersion.mBuildNumber << std::endl << std::flush;
		
		// Create DAL instance
		try
		{
			mInstance = Audinate::DAL::createInstance(mDal, mConfig);

			if (!mInstance->isDeviceActivated())
			{
				mLog << "DAL device is not activated" << std::endl << std::flush;
			}
			else
			{
				mLog << "DAL device is activated" << std::endl << std::flush;
			}
		}
		catch (const Audinate::DAL::DalException& exception)
		{
			mLog << exception.getErrorDescription() << "\t(" << exception.getErrorName() << ")" << std::endl << std::flush;
			g_running = false;
			mDal = nullptr;
			return -1;
		}

		std::shared_ptr<Audinate::DAL::Instance> instance = mInstance;
		mInstance->setEventFn([instance](const Audinate::DAL::InstanceEvent& ev)->void {
			handleEvent(*instance, ev);
			});

		if (monitor)
		{
			mInstance->setMonitoringFn(handleMonitoringEvent);
		}

		setupAudioTransfer();

		return 0;
	}

	static void sig_handler(int sig)
	{
		(void)sig;
		g_running = false;
	}

	static std::vector<Audinate::DAL::Protocol> protocols =
	{
		Audinate::DAL::Protocol::Arcp,
		Audinate::DAL::Protocol::ArcpLocal,
		Audinate::DAL::Protocol::Dbcp,
		Audinate::DAL::Protocol::Cmcp,
		Audinate::DAL::Protocol::ConmonChannels,
		Audinate::DAL::Protocol::ConmonClient,
		Audinate::DAL::Protocol::DomainClientProxy,
		Audinate::DAL::Protocol::AudioBase,
		Audinate::DAL::Protocol::WebSocket
	#ifdef _WIN32
		, Audinate::DAL::Protocol::MdnsClient
	#endif
	};

	void DalAppBase::run()
	{
		if (!mInstance)
		{
			mLog << "DAL instance has not been created" << std::endl << std::flush;
			return;
		}

		try
		{
			mInstance->start();
		}
		catch (const Audinate::DAL::DalException& exception)
		{
			mLog << exception.getErrorDescription() << "\t(" << exception.getErrorName() << ")" << std::endl << std::flush;
			g_running = false;
		}

		mLog << "\nSocket Descriptor validation: ";
		for (auto iter : protocols)
		{
			auto configSd = mConfig.getProtocolSocketDescriptor(iter);
			auto actualSd = mInstance->getProtocolSocketDescriptor(iter);
			mLog << "Protocol" << Audinate::DAL::toString(iter) << " config=" << Audinate::DAL::toString(configSd) << " actual=" << Audinate::DAL::toString(actualSd) << std::endl << std::flush;
		}

	}

	void DalAppBase::stop()
	{
		g_running = false;
	}

	bool DalAppBase::getAudioProperties(Audinate::DAL::AudioProperties& properties)
	{
		std::shared_ptr<Audinate::DAL::Audio> audio = mInstance->getAudio();
		if (!audio)
		{
			return false;
		}

		properties = audio->getProperties();

		return true;
	}

#define LATENCY_SAMPLES 480

	//This function sets up the audio transfer function.
	void DalAppBase::setupAudioTransfer()
	{
		assert(mInstance);

		if (mInstance->getInstanceState() != Audinate::DAL::InstanceState::Stopped)
		{
			mLog << "DAL instance should be stopped before updating audio transfer" << std::endl << std::flush;
			return;
		}

		std::shared_ptr<Audinate::DAL::Audio> audio = mInstance->getAudio();
		assert(audio);

		//This example uses the available channel counts to setup transfer callback if activated.
		//Note: The minimum of RX and TX channels is used.
		//If not activated, transfer callback is set to null.
		if (mInstance->isDeviceActivated())
		{
			Audinate::DAL::AudioProperties properties = audio->getProperties();

			unsigned int numChannels = (std::min)
				((int)properties.mRxActivatedChannelCount, (int)properties.mTxActivatedChannelCount);

			unsigned int latencySamples = LATENCY_SAMPLES;

			mLog << "SetupAudioTransfer function...Instance state" << Audinate::DAL::toString(mInstance->getInstanceState()) << std::endl << std::flush;
			
			audio->setTransferFn([properties, numChannels, latencySamples, this](const Audinate::DAL::AudioTransferParameters& params)->void {
				mTransferFn(properties, params, numChannels, latencySamples);
				});
		}
		else
		{
			audio->setTransferFn(nullptr);
		}
	}

	bool DalAppBase::isDeviceActivated()
	{
		return componentApecRunning;
	}

	//This function stops the DAL instance, resets the audio transfer
	//with updated properties and starts the DAL instance.
	void DalAppBase::restartDalInstance()
	{
		assert(mInstance);

		try
		{
			mInstance->stop();

			setupAudioTransfer();

			mInstance->start();
		}
		catch (const Audinate::DAL::DalException& exception)
		{
			mLog << exception.getErrorDescription() << "\t(" << exception.getErrorName() << ")" << std::endl << std::flush;
		}
	}

	//This function stops the DAL instance
	void DalAppBase::stopDalInstance()
	{
		assert(mInstance);

		try
		{
			mInstance->stop();
		}
		catch (const Audinate::DAL::DalException& exception)
		{
			mLog << exception.getErrorDescription() << "\t(" << exception.getErrorName() << ")" << std::endl << std::flush;
			return;
		}
	}

	//This function resets the DAL instance
	void DalAppBase::resetDalInstance()
	{
		assert(mInstance);

		std::shared_ptr<Audinate::DAL::Audio> audio = mInstance->getAudio();
		assert(audio);

		// Lambda handler have shared pointers to audio / instance
		audio->setTransferFn(nullptr);
		audio = nullptr;

		mInstance->setMonitoringFn(nullptr);
		mInstance->setEventFn(nullptr);
		mInstance = nullptr;
		mDal = nullptr;
	}
	//
	// Copyright © 2021-2022 Audinate Pty Ltd ACN 120 828 006 (Audinate). All rights reserved.
	//
}