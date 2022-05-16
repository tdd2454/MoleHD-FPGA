#include "data_tokenize.h"

using namespace std;

// int input[39] = {42, 67, 40, 61, 79, 41, 91, 67, 64, 72, 93, 40, 67, 67, 67, 67, 78, 67, 40, 61, 79, 41, 79, 67, 67, 79, 67, 41, 78, 67, 40, 61, 79, 41, 79, 67, 67, 79, 67};

vector<vector<int>> input = {{42, 67, 40, 61, 79, 41, 91, 67, 64, 72, 93, 40, 67, 67, 67, 67, 78, 67, 40, 61, 79, 41, 79, 67, 67, 79, 67, 41, 78, 67, 40, 61, 79, 41, 79, 67, 67, 79, 67},
							{67, 91, 78, 72, 50, 43, 93, 91, 67, 64, 72, 93, 49, 67, 67, 91, 67, 64, 72, 93, 40, 99, 50, 99, 49, 99, 99, 99, 99, 50, 41, 99, 51, 99, 99, 99, 40, 99, 40, 99, 51, 41, 67, 108, 41, 67, 108},
							{67, 67, 49, 40, 91, 67, 64, 64, 72, 93, 40, 78, 50, 91, 67, 64, 72, 93, 40, 83, 49, 41, 91, 67, 64, 64, 72, 93, 40, 67, 50, 61, 79, 41, 78, 51, 67, 40, 61, 79, 41, 91, 67, 64, 72, 93, 40, 91, 78, 72, 50, 43, 93, 67, 51, 40, 67, 41, 67, 41, 99, 52, 99, 99, 99, 99, 99, 52, 41, 67, 40, 61, 79, 41, 91, 79, 45, 93, 41, 67}};

void print_value(int *input);

int main()
{
	hls::stream<DATA> input_dma("input_dma");
	hls::stream<DATA> output_dma("output_dma");

	data_t in_val;

	for (int i = 0; i < NUM_TOKEN; i++)
	{
		in_val.data = lookup_char[i];
		if (i == NUM_TOKEN - 1)
			in_val.last = 1;
		else
			in_val.last = 0;
		input_dma.write(in_val);
	}

	for (int i = 0; i < CHUNK_NUM; i++)
	{
		for (int j = 0; j < CHUNK_SIZE; j++)
		{
			in_val.data = lookup_HV[i][j];
			if (i == CHUNK_NUM - 1 && j == CHUNK_SIZE - 1)
				in_val.last = 1;
			else
				in_val.last = 0;
			input_dma.write(in_val);
		}
	}

	// for (int i = 0; i < 39; i++)
	// {
	// 	in_val.data = input[i];
	// 	if (i == 39 - 1)
	// 		in_val.last = 1;
	// 	else
	// 		in_val.last = 0;
	// 	input_dma.write(in_val);
	// }

	// hv_core(input_dma, output_dma, run, state, cur_size);

	for (int i = 0; i < input.size(); i++)
	{
		// cout << "Size of input " << sizeof(input) << endl;
		// cout << "Size of subinput " << sizeof(input[i]) << endl;
		for (int j = 0; j < input[i].size(); j++)
		{
			in_val.data = input[i][j];
			// cout << in_val.data << ", \t";
			if (j == input[i].size() - 1)
				in_val.last = 1;
			else
				in_val.last = 0;
			input_dma.write(in_val);
		}
		cout << endl;
	}

	int run, state, cur_size;
	run = 3;
	hv_core(input_dma, output_dma, run, state, cur_size);

	data_t out_val;

	for (int i = 0; i < NUM_TOKEN; i++)
	{
		out_val = output_dma.read();
		// cout << out_val.data << endl;
		if (out_val.last == 1)
		{
			cout << "End of stream 1" << endl;
			break;
		}
	}
	for (int i = 0; i < CHUNK_NUM; i++)
	{
		for (int j = 0; j < CHUNK_SIZE; j++)
		{
			out_val = output_dma.read();
			if (i == CHUNK_NUM - 1 && j == CHUNK_SIZE - 1)
			{
				cout << "End of stream 2" << endl;
				break;
			}
		}
	}

	for (int i = 0; i < input.size(); i++)
	{
		for (int j = 0; j < input[i].size(); j++)
		{
			out_val = output_dma.read();
			if (out_val.last == 1)
			{
				cout << "End of stream 3 " << endl;
				break;
			}	
		}
	}

	for (int l = 0; l < 3; l++)
	{
		for (int i = 0; i < CHUNK_NUM; i++)
		{
			for (int j = 0; j < CHUNK_SIZE; j++)
			{
				out_val = output_dma.read();
				cout << out_val.data << ", \t";
				if (out_val.last == 1)
				{
					cout << "End of stream 4" << endl;
					break;
				}
			}
		}
	}

	return 0;
}

void print_value(int *input)
{
	for (int i = 0; i < CHUNK_NUM; i++)
	{
		printf("%d , \t", input[i]);
	}
}
