#include "data_tokenize.h"

using namespace std;

#define MAX_IN_LEN 350
#define LOOKUP_KEY_LEN 256

void load(hls::stream<DATA> &input_dma, hls::stream<DATA> &output_dma, int max_len, ap_int<32> *data, int &size, data_t &format)
{
	data_t in_val;
	int i = 0;
	// KEY0:
	for (; i < max_len; i++)
	{
#pragma HLS PIPELINE
		in_val = input_dma.read();
		data[i] = in_val.data;

		if (in_val.last == 1)
		{
			break;
		}
	}
	size = i + 1;
	format = in_val;
}

void load_2d(hls::stream<DATA> &input_dma, hls::stream<DATA> &output_dma, ap_int<32> data[CHUNK_NUM][CHUNK_SIZE])
{
	data_t in_val;
	// KEY0:
	for (int i = 0; i < CHUNK_NUM; i++)
	{
#pragma HLS PIPELINE
		for (int j = 0; j < CHUNK_SIZE; j++)
		{
			in_val = input_dma.read();
			data[i][j] = in_val.data;

			if (in_val.last == 1)
			{
				break;
			}
		}
	}
}

void encode(ap_int<32> *input, ap_int<32> lookup_key[NUM_TOKEN], ap_int<32> lookup_value[CHUNK_NUM][CHUNK_SIZE],
			ap_int<32> result_HV[CHUNK_NUM][CHUNK_SIZE], int size)
{
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < NUM_TOKEN; j++)
		{
			if (input[i] == lookup_key[j])
			{
				int position = j;
				// cout << position << " value input " << input[i] << " value key " << lookup_key[j] << endl;
				for (int k = 0; k < CHUNK_NUM; k++)
				{
#pragma HLS PIPELINE II=1
					position = position % CHUNK_NUM;

					for (int l = 0; l < CHUNK_SIZE; l++)
					{
#pragma HLS ARRAY_RESHAPE variable=lookup_value complete dim=2
#pragma HLS ARRAY_RESHAPE variable=result_HV complete dim=2
						if (lookup_value[position][l] == 1)
							result_HV[k][l] = result_HV[k][l] + 1;
						else
							result_HV[k][l] = result_HV[k][l] - 1;
					}
					position = position + 1;
				}
			}
		}
	}
}

void return_val(hls::stream<DATA> &output_dma, ap_int<32> data[CHUNK_NUM][CHUNK_SIZE], data_t &format)
{

	data_t out_val;

	for (int i = 0; i < CHUNK_NUM; i++)
	{
		for (int j = 0; j < CHUNK_SIZE; j++)
		{
			out_val.data = data[i][j];
			out_val.keep = format.keep;
			out_val.strb = format.strb;
			out_val.user = format.user;
			out_val.id = format.id;
			out_val.dest = format.dest;

			if ((i == CHUNK_NUM - 1) && (j == CHUNK_SIZE - 1))
			{
				out_val.last = 1;
			}
			else
			{
				out_val.last = 0;
			}

			output_dma.write(out_val);
			// cout << " i = " << i << "j = "<< j << "last " << out_val.last << endl;
		}
	}
}

void hv_core(hls::stream<DATA> &input_dma,
			 hls::stream<DATA> &output_dma,
			 int run,
			 // DEBUG ONLY
			 int &state,
			 int &cur_size)
{

#pragma HLS INTERFACE s_axilite port = return bundle = CRTL_BUS
#pragma HLS INTERFACE s_axilite port = run bundle = CRTL_BUS
#pragma HLS INTERFACE s_axilite port = state bundle = CRTL_BUS
#pragma HLS INTERFACE s_axilite port = cur_size bundle = CRTL_BUS

#pragma HLS INTERFACE axis port = input_dma
#pragma HLS INTERFACE axis port = output_dma

	static ap_int<32> input[MAX_IN_LEN];
#pragma HLS RESOURCE variable = input core = RAM_1P_BRAM
#pragma HLS ARRAY_PARTITION variable = input complete dim = 1

	static ap_int<32> lookup_key[NUM_TOKEN];
	// static ap_int<32> lookup_key[LOOKUP_KEY_LEN];
#pragma HLS RESOURCE variable = lookup_key core = RAM_1P_BRAM
#pragma HLS ARRAY_PARTITION variable = lookup_key complete dim = 1

	static ap_int<32> lookup_value[CHUNK_NUM][CHUNK_SIZE];
#pragma HLS RESOURCE variable = lookup_value core = RAM_1P_BRAM
#pragma HLS ARRAY_PARTITION variable = lookup_value complete dim = 1

	static ap_int<32> result_HV[CHUNK_NUM][CHUNK_SIZE];

	state = 0;
	int start = 0;
	data_t format;

	// Load token characters
	load(input_dma, output_dma, NUM_TOKEN, lookup_key, cur_size, format);
	cout << "Receive size: " << cur_size << endl;
	state = 1;
	// Load token hypervector value
	load_2d(input_dma, output_dma, lookup_value);
	state = 2;

	// Load input module
	while (start < run)
	{
		for (int i = 0; i < CHUNK_NUM; i++)
		{
			for (int j = 0; j < CHUNK_SIZE; j++)
			{
				result_HV[i][j] = 0;
			}
		}

		load(input_dma, output_dma, MAX_IN_LEN, input, cur_size, format);
		cout << "Receive size: " << cur_size << endl;
		state = 3;

		encode(input, lookup_key, lookup_value, result_HV, cur_size);

		cout << endl;
		return_val(output_dma, result_HV, format);
		state = 5;

		start = start + 1;
	}
}
