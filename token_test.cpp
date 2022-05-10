#include "data_tokenize.h"

using namespace std;

FPGA_DATA input[in_len] = {'*', 'C', '(', '=', 'O', ')', '[', 'C', '@', 'H', ']', '(', 'C', 'C', 'C', 'C', 'N', 'C', '(', '=', 'O', ')', 'O', 'C', 'C', 'O', 'C', ')', 'N', 'C', '(', '=', 'O', ')', 'O', 'C', 'C', 'O', 'C'};

void print_value(int *input);

int main()
{
	/*
		hls::stream<DMA_DATA> input_dma_char("input_dma_char");
		hls::stream<HV_DATA_I> input_lookup_data("input_lookup_data");
		hls::stream<DMA_DATA> input_lookup_char("input_lookup_char");
		hls::stream<HV_DATA_O> output_dma_O("output_dma_Out");
		hls::stream<HV_DATA_O> output_dma_I("output_dma_In");

		hls::stream<DMA_DATA> check_DMA_in("check_DMA_in");
		hls::stream<HV_DATA_I> check_HV_in("check_HV_in");
		hls::stream<DMA_DATA> check_Data_in("check_Data_in");
		int state = 0;

		DMA_DATA lookupChar;
		for (int i = 0; i < NUM_TOKEN; i++)
		{
			lookupChar.data = lookup_char[i];
			if (i == NUM_TOKEN - 1)
				lookupChar.last = 1;
			else
				lookupChar.last = 0;
			input_lookup_char.write(lookupChar);
		}

		HV_DATA_I lookupStream;
		for (int i = 0; i < CHUNK_NUM; i++)
		{

			for (int j = 0; j < CHUNK_SIZE; j++)
			{
				lookupStream.data = lookup_HV[i][j];
				if (i == CHUNK_NUM - 1 && j == CHUNK_SIZE - 1)
				{
					lookupStream.last = 1;
				}
				else
				{
					lookupStream.last = 0;
				}
				input_lookup_data.write(lookupStream);
			}
		}

		HV_DATA_O Output_In;

		for (int i = 0; i < CHUNK_NUM * CHUNK_SIZE; i++)
		{
			Output_In.data = 3;

			if (i == 10 - 1)
				Output_In.last = 1;
			else
				Output_In.last = 0;
			output_dma_I.write(Output_In);
		}

		DMA_DATA dataStream;
		int IFM_len = 39;
		for (int i = 0; i < IFM_len; i++)
		{
			dataStream.data = input[i];

			if (i == IFM_len - 1)
				dataStream.last = 1;
			else
				dataStream.last = 0;
			input_dma_char.write(dataStream);
		}

		CREATE_HV(input_dma_char, output_dma_I, input_lookup_char, input_lookup_data, output_dma_O, check_DMA_in, check_HV_in, check_Data_in, state, IFM_len, 0);

		DMA_DATA dataStream;

		for(int i = 0; i < IFM_len; i++){
			dataStream.data = input[i];

			if(i == IFM_len-1)
				dataStream.last = 1;
			else
				dataStream.last = 0;
			input_dma_char.write(dataStream);
		}

		state = 2;
		CREATE_HV(input_dma_char, output_dma_I, input_lookup_char, input_lookup_data, output_dma_O, check_DMA_in, check_HV_in, check_Data_in, state, IFM_len, 0);

		state = 3;
		CREATE_HV(input_dma_char, output_dma_I, input_lookup_char, input_lookup_data, output_dma_O, check_DMA_in, check_HV_in, check_Data_in, state, IFM_len, 1);
		CREATE_HV(input_dma_char, input_lookup_char, input_lookup_data, output_dma_O, check_DMA_in, check_HV_in, state, IFM_len, 2);
		CREATE_HV(input_dma_char, input_lookup_char, input_lookup_data, output_dma_O, check_DMA_in, check_HV_in, state, IFM_len, 3);
		CREATE_HV(input_dma_char, input_lookup_char, input_lookup_data, output_dma_O, check_DMA_in, check_HV_in, state, IFM_len, 4);
		CREATE_HV(input_dma_char, input_lookup_char, input_lookup_data, output_dma_O, check_DMA_in, check_HV_in, state, IFM_len, 5);

		HV_DATA_O output_HV;
		int output_check[CHUNK_NUM*CHUNK_SIZE];
		for(int i = 0; i < CHUNK_NUM*CHUNK_SIZE; i++){
			output_HV = output_dma_O.read();
			cout << output_HV.data << endl;
			if(output_HV.last == 1){
				break;
			}
			else{
				output_check[i] = output_HV.data;

			}
		}

		print_value(output_check);
	*/
	hls::stream<DATA> input_dma("input_dma");
	hls::stream<DATA> output_dma("output_dma");

	data_t in_val;

	for (int i = 0; i < NUM_TOKEN; i++)
	{
		in_val.data = i;
		if (i == NUM_TOKEN - 1)
			in_val.last = 1;
		else
			in_val.last = 0;
		input_dma.write(in_val);
	}

	for(int i = 0; i < CHUNK_NUM; i++){
		for(int j = 0; j < CHUNK_SIZE; j++){
			in_val.data = lookup_HV[i][j];
			if(i == CHUNK_NUM-1 && j == CHUNK_SIZE-1)
				in_val.last = 1;
			else
				in_val.last = 0;
			input_dma.write(in_val);
		}
	}

	for (int i = 0; i < 30; i++)
	{
		in_val.data = i;
		if (i == 30 - 1)
			in_val.last = 1;
		else
			in_val.last = 0;
		input_dma.write(in_val);
	}

	int state, cur_size;
	hv_core(input_dma, output_dma, state, cur_size);

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

	for(int i = 0; i < CHUNK_NUM; i++){
		for(int j = 0; j < CHUNK_SIZE; j++){
			out_val = output_dma.read();
			if(i == CHUNK_NUM-1 && j == CHUNK_SIZE-1)
			{
				cout << "End of stream 2" << endl;
				break;
			}
		}
	}

	for (int i = 0; i < 30; i++)
	{
		out_val = output_dma.read();
		// cout << out_val.data << endl;
		if (out_val.last == 1)
		{
			cout << "End of stream 3" << endl;
			break;
		}
	}

	for (int i = 0; i < CHUNK_NUM; i++)
	{
		for (int j = 0; j < CHUNK_SIZE; j++)
		{
			out_val = output_dma.read();
			cout << out_val.data << " last " << out_val.last << endl;
			if (out_val.last == 1)
			{
				cout << "End of stream 4" << endl;
				break;
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
