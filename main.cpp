#include <cstdio>	//stdandard output
#include <cstdlib>

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Formats.hpp>

#include <string>	// std::string
#include <vector>	// std::vector<...>
#include <map>		// std::map< ... , ... >

#include <iostream>


int main()
{

	// -1. MY VARS

	unsigned samp_rate = 240000;
	unsigned freq = 27195700;

	// 0. enumerate devices (list all devices' information)
	SoapySDR::KwargsList results = SoapySDR::Device::enumerate();
	SoapySDR::Kwargs::iterator it;
	unsigned where = 0;

	for( int i = 0; i < results.size(); ++i)

	{
		printf("Found device #%d: ", i);
		for( it = results[i].begin(); it != results[i].end(); ++it)
		{
			printf("%s = %s\n", it->first.c_str(), it->second.c_str());
			if(it->second == "RTL2838UHIDIR"){
				where = i;
			}
		}
		printf("\n");
	}

	// 1. create device instance
	
	//	1.1 set arguments
	//		args can be user defined or from the enumeration result
	//		We use first results as args here:
	SoapySDR::Kwargs args = results[where];

	//	1.2 make device
	SoapySDR::Device *sdr = SoapySDR::Device::make(args);

	if( sdr == NULL )
	{
		fprintf(stderr, "SoapySDR::Device::make failed\n");
		return EXIT_FAILURE;
	}

	// 2. query device info
	std::vector< std::string > str_list;	//string list

	//	2.1 antennas
	str_list = sdr->listAntennas( SOAPY_SDR_RX, 0);
	printf("Rx antennas: ");
	for(int i = 0; i < str_list.size(); ++i)
		printf("%s,", str_list[i].c_str());
	printf("\n");

	//	2.2 gains
	str_list = sdr->listGains( SOAPY_SDR_RX, 0);
	printf("Rx Gains: ");
	for(int i = 0; i < str_list.size(); ++i)
		printf("%s, ", str_list[i].c_str());
	printf("\n");

	//	2.3. ranges(frequency ranges)
	SoapySDR::RangeList ranges = sdr->getFrequencyRange( SOAPY_SDR_RX, 0);
	printf("Rx freq ranges: ");
	for(int i = 0; i < ranges.size(); ++i)
		printf("[%g Hz -> %g Hz], ", ranges[i].minimum(), ranges[i].maximum());
	printf("\n");

	// 3. apply settings
	sdr->setSampleRate( SOAPY_SDR_RX, 0, samp_rate);

	sdr->setFrequency( SOAPY_SDR_RX, 0, freq);

	// 4. setup a stream (complex floats)
	SoapySDR::Stream *rx_stream = sdr->setupStream( SOAPY_SDR_RX, SOAPY_SDR_CF32);
	if( rx_stream == NULL)
	{
		fprintf( stderr, "Failed\n");
		SoapySDR::Device::unmake( sdr );
		return EXIT_FAILURE;
	}
	sdr->activateStream( rx_stream, 0, 0, 0);

	// 5. create a re-usable buffer for rx samples
	std::complex<float> buff[1024];

	for( int i = 0; i < 100000; ++i)
	{
		void *buffs[] = {buff};
		int flags;
		long long time_ns;
		int ret = sdr->readStream( rx_stream, buffs, 1024, flags, time_ns, 5000);

		//My print statement
		std::cout << time_ns << "," << ret << "\n";
	}

	// 7. shutdown the stream
	sdr->deactivateStream( rx_stream, 0, 0);	//stop streaming
	sdr->closeStream( rx_stream );

	// 8. cleanup device handle
	SoapySDR::Device::unmake( sdr );
	printf("Done\n");

	return EXIT_SUCCESS;
}
