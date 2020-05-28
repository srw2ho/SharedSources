#include "pch.h"
#include "GPIODriver.h"
#include "GPIOPin.h"
#include "GPIOInOut.h"







namespace GPIODriver
{
	bool GPIOPin::doparse(mpack_reader_t* reader) {

		Lock();
		char szBuf[50];

		mpack_expect_cstr(reader, szBuf, sizeof(szBuf));
		size_t len = strlen(szBuf);
		if (len != strlen(this->m_PinName.c_str())) {
			this->m_PinName = szBuf;
		}

		//strcpy(this->m_PinName, szBuf);

		this->m_ActivateProcessing = mpack_expect_bool(reader);

		this->m_ActivateOutputProcessing = mpack_expect_bool(reader);

		this->m_PinValue = mpack_expect_double(reader);
		this->m_InitValue = mpack_expect_double(reader);
		this->m_SetValue = mpack_expect_double(reader);

		this->m_PulseTimeinms = mpack_expect_double(reader);
		this->m_ActTimeinNanos = mpack_expect_u64(reader);

		//	mpack_error_t error = mpack_reader_destroy(reader);
		mpack_error_t error = mpack_reader_error(reader);

		if (error != mpack_ok) {
			UnLock();
			return false;
		}

		UnLock();
		return true;
	}

	bool GPIOHCSR04::doparse(mpack_reader_t* reader) {
		Lock();
		GPIOPin::doparse(reader);

		this->m_TriggerPinNumber = mpack_expect_uint(reader);

		//mpack_error_t error = mpack_reader_destroy(reader);
		mpack_error_t error = mpack_reader_error(reader);
		if (error != mpack_ok) {
			UnLock();
			return false;
		}
		UnLock();
		return true;

	}


	bool GPIOPWMOutputPin::doparse(mpack_reader_t* reader) {
		Lock();
		GPIOPin::doparse(reader);

		this->m_Frequency = mpack_expect_uint(reader);

		//mpack_error_t error = mpack_reader_destroy(reader);
		mpack_error_t error = mpack_reader_error(reader);
		if (error != mpack_ok) {
			UnLock();
			return false;
		}
		UnLock();
		return true;

	}

	bool BME280Sensor::doparse(mpack_reader_t* reader) {
		Lock();
		GPIOPin::doparse(reader);

		this->m_pressure = mpack_expect_double(reader);
		this->m_temperature = mpack_expect_double(reader);
		this->m_humidity = mpack_expect_double(reader);

		//mpack_error_t error = mpack_reader_destroy(reader);
		mpack_error_t error = mpack_reader_error(reader);
		if (error != mpack_ok) {
			UnLock();
			return false;
		}
		UnLock();
		return true;

	}

	bool BME680Sensor::doparse(mpack_reader_t* reader) {
		Lock();
		GPIOPin::doparse(reader);


		this->m_iaq = mpack_expect_double(reader);
		this->m_iaq_accuracy = mpack_expect_int(reader);

		this->m_temperature = mpack_expect_double(reader);
		this->m_humidity = mpack_expect_double(reader);
		this->m_pressure = mpack_expect_double(reader);

		this->m_raw_temperature = mpack_expect_double(reader);
		this->m_raw_humidity = mpack_expect_double(reader);

		this->m_gas = mpack_expect_double(reader);
		this->m_bsec_status = mpack_expect_int(reader);
		this->m_static_iaq = mpack_expect_double(reader);
		this->m_co2_equivalent = mpack_expect_double(reader);
		this->m_breath_voc_equivalent = mpack_expect_double(reader);

		//mpack_error_t error = mpack_reader_destroy(reader);
		mpack_error_t error = mpack_reader_error(reader);
		if (error != mpack_ok) {
			UnLock();
			return false;
		}
		UnLock();
		return true;

	}

	bool ADCMT3620::doparse(mpack_reader_t* reader) {
		Lock();
		GPIOPin::doparse(reader);

		mpack_error_t error = mpack_reader_error(reader);
		if (error != mpack_ok) {
			UnLock();
			return false;
		}
		UnLock();
		return true;

	}
	bool PWMMT3620::doparse(mpack_reader_t* reader) {
		Lock();
		GPIOPin::doparse(reader);

		this->m_ChannelNos = mpack_expect_u16(reader);

		size_t count = mpack_expect_array_max(reader, this->m_ChannelNos);
		PWMChannel* pwmChannel = (PWMChannel*)&this->m_PWMChannels[0];

		for (size_t i = 0; i < count; ++i) {
			uint32_t len = mpack_expect_bin(reader);
			mpack_read_bytes(reader,(char*) &pwmChannel->m_UserPWMState, len);
			mpack_done_bin(reader);
			pwmChannel++;
		}

		mpack_done_array(reader);

		mpack_error_t error = mpack_reader_error(reader);
		if (error != mpack_ok) {
			UnLock();
			return false;
		}
		UnLock();
		return true;

	}

	bool GPIOInOut::parse_mpegandgetObject(mpack_reader_t* reader) {

		uint8_t type = mpack_expect_u8(reader);
		unsigned int PinNumber = mpack_expect_u16(reader);

		GPIOPin* retValue = this->getGPIOPinByPinNrandType((GPIOTyp)type, PinNumber);
		bool bParseOK;
		if (retValue == nullptr) {

			switch (type) {
			case GPIOTyp::output: {
				GPIOOutputPin* pGPIOPin = nullptr;
				pGPIOPin = new GPIOOutputPin(this->m_pGPIOEventPackageQueue, PinNumber, 0);
				bParseOK = pGPIOPin->doparse(reader);
#if !GPIOCLIENT_USING
				if (bParseOK) {
					bool InitOK = pGPIOPin->Init(m_GPIOController);
					if (!InitOK)
					{
						delete pGPIOPin;
						retValue = nullptr;
					}
					else retValue = pGPIOPin;
				}

#endif
			}

				break;
			case GPIOTyp::input: {
				GPIOInputPin* pGPIOPin = nullptr;
				pGPIOPin = new GPIOInputPin(this->m_pGPIOEventPackageQueue, PinNumber,0);
				bParseOK = pGPIOPin->doparse(reader);
#if !GPIOCLIENT_USING
				if (bParseOK) {
					bool InitOK = pGPIOPin->Init(m_GPIOController);
					if (!InitOK)
					{
						delete pGPIOPin;
						retValue = nullptr;
					}
					else retValue = pGPIOPin;
				}

#endif
				break;
			}
			case GPIOTyp::PWM: {
				GPIOPWMOutputPin* pGPIOPin = nullptr;
				pGPIOPin = new GPIOPWMOutputPin(this->m_pGPIOEventPackageQueue, PinNumber,0);
				bParseOK = pGPIOPin->doparse(reader);
#if !GPIOCLIENT_USING
				if (bParseOK) {
					bool InitOK = pGPIOPin->Init(m_PWMController, pGPIOPin->getFrequency());
					if (!InitOK)
					{
						delete pGPIOPin;
						retValue = nullptr;
					}
					else retValue = pGPIOPin;
				}

#endif
				break;
			}
			case GPIOTyp::PWM9685: {
				GPIOPWMPWM9685OutputPin* pGPIOPin = nullptr;
				pGPIOPin = new GPIOPWMPWM9685OutputPin(this->m_pGPIOEventPackageQueue, PinNumber, 0);
				bParseOK = pGPIOPin->doparse(reader);
#if !GPIOCLIENT_USING
				if (bParseOK) {
					bool InitOK = pGPIOPin->Init(m_PWMController, pGPIOPin->getFrequency());
					if (!InitOK)
					{
						delete pGPIOPin;
						retValue = nullptr;
					}
					else retValue = pGPIOPin;
				}

#endif
				break;
			}
			case GPIOTyp::PWM_MT3620: {
				PWMMT3620* pGPIOPin = nullptr;
				pGPIOPin = new PWMMT3620 (this->m_pGPIOEventPackageQueue, PinNumber, 3,0,0,0);
				bParseOK = pGPIOPin->doparse(reader);
#if !GPIOCLIENT_USING
				if (bParseOK) {
					retValue = pGPIOPin;
				}

#endif
				break;

			}

			case GPIOTyp::ADC_MT3620: {
				ADCMT3620* pGPIOPin = nullptr;
				pGPIOPin = new ADCMT3620(this->m_pGPIOEventPackageQueue, PinNumber, 0);
				bParseOK = pGPIOPin->doparse(reader);
#if !GPIOCLIENT_USING
				if (bParseOK) {
					retValue = pGPIOPin;
				}

#endif
				break;
			}
									

			case GPIOTyp::BME680: {
				BME680Sensor* pGPIOPin = nullptr;
				pGPIOPin = new BME680Sensor(this->m_pGPIOEventPackageQueue, PinNumber, 0);
				bParseOK = pGPIOPin->doparse(reader);
#if !GPIOCLIENT_USING
				if (bParseOK) {
					retValue = pGPIOPin;
				}
#endif
				break;

			}

			case GPIOTyp::BME280: {
			//	BME280Sensor* pGPIOPin = nullptr;
			//	pGPIOPin = new BME280Sensor(this->m_pGPIOEventPackageQueue, PinNumber, 0);
				bParseOK = true;
#if !GPIOCLIENT_USING
				if (bParseOK) {

					CreateBME280External(this, (int)PinNumber);
					retValue = nullptr;
				}

#endif
				break;
			}

			case GPIOTyp::HC_SR04: {
				GPIOHCSR04* pGPIOPin = nullptr;
				pGPIOPin = new GPIOHCSR04(this->m_pGPIOEventPackageQueue, PinNumber,0, 0);
				bParseOK = pGPIOPin->doparse(reader);
#if !GPIOCLIENT_USING
				if (bParseOK) {
					bool InitOK = pGPIOPin->Init(this->m_GPIOController);
					if (!InitOK)
					{
						delete pGPIOPin;
						retValue = nullptr;
					}
					else retValue = pGPIOPin;

				}

#endif
				break;
			}

			}
			if (retValue != nullptr) {
				retValue->setActivateProcessing(true);
				this->addGPIOPin(retValue);
			}
		}
		else {
			bParseOK = retValue->doparse(reader);
			retValue->setActivateProcessing(true);
		}

		return bParseOK;
	}

	bool GPIOInOut::parse_mpackObjects(std::vector<byte>& arr) {
		bool doParse = true;
		size_t dataIdx = 0;

		mpack_reader_t mpackreader;
		char* charPtr = (char*)&arr[0];

		mpack_reader_init_data(&mpackreader, charPtr, arr.size());
		bool retVal = false;
		Lock();

		while (doParse) {

			doParse = parse_mpegandgetObject(&mpackreader);
			if (doParse) {
				retVal = true;
				doParse = (mpack_reader_remaining(&mpackreader, NULL) > 0);
			}

		}
		mpack_error_t error = mpack_reader_destroy(&mpackreader);
		if (error != mpack_ok) {

			retVal = false;
		}
		else {
			DoProcessGPIOCommand();
		}


		UnLock();
		return retVal;
	}
}