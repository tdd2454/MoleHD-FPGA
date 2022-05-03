#include "data_tokenize.h"

using namespace std;

FPGA_DATA input[in_len] = {'*','C','(','=','O',')','[','C','@','H',']','(','C','C','C','C','N','C','(','=','O',')','O','C','C','O','C',')','N','C','(','=','O',')','O','C','C','O','C'};


void CREATE_HV(hls::stream<DMA_DATA> &input_dma_char,
		hls::stream<DMA_DATA> &input_lookup_char,
		hls::stream<HV_DATA_I> &input_lookup_data,	
		hls::stream<HV_DATA_O> &output_dma_O,
		hls::stream<DMA_DATA> &check_DMA_in,
		hls::stream<HV_DATA_I> &check_HV_in,
		int state,
		int IFM_len);

void print_value(int* input);

int main(){
	hls::stream<DMA_DATA> input_dma_char("input_dma_char");
	hls::stream<HV_DATA_I> input_lookup_data("input_lookup_data");
	hls::stream<DMA_DATA> input_lookup_char("input_lookup_char");
	hls::stream<HV_DATA_O> output_dma_O("output_dma_Out");

	hls::stream<DMA_DATA> check_DMA_in("check_DMA_in");
	hls::stream<HV_DATA_I> check_HV_in("check_HV_in");
	int state = 0;

	DMA_DATA lookupChar;
	for(int i = 0; i < NUM_TOKEN; i++){
		lookupChar.data = lookup_char[i];
		if(i == NUM_TOKEN-1)
			lookupChar.last = 1;
		else
			lookupChar.last = 0;
		input_lookup_char.write(lookupChar);
	}

	HV_DATA_I lookupStream;
	for(int i = 0; i < CHUNK_NUM; i++){

		for(int j = 0; j < CHUNK_SIZE; j++){
			lookupStream.data = lookup_HV[i][j];
			if(i == CHUNK_NUM-1 && j == CHUNK_SIZE-1){
				lookupStream.last = 1;
			}
			else{
				lookupStream.last = 0;
			}
			input_lookup_data.write(lookupStream);
		}
	}

	int IFM_len =39;
	state = 1;
	CREATE_HV(input_dma_char, input_lookup_char, input_lookup_data, output_dma_O, check_DMA_in, check_HV_in, state, IFM_len);

	DMA_DATA dataStream;

	for(int i = 0; i < in_len; i++){
		dataStream.data = input[i];
		
		if(i == in_len-1)
			dataStream.last = 0;
		else
			dataStream.last = 1;
		input_dma_char.write(dataStream);
	}

	state = 2;
	CREATE_HV(input_dma_char, input_lookup_char, input_lookup_data, output_dma_O, check_DMA_in, check_HV_in, state, IFM_len);

	state = 3;
	CREATE_HV(input_dma_char, input_lookup_char, input_lookup_data, output_dma_O, check_DMA_in, check_HV_in, state, IFM_len);

	HV_DATA_O output_HV;
	int output_check[CHUNK_NUM*CHUNK_SIZE];
	for(int i = 0; i < CHUNK_NUM*CHUNK_SIZE; i++){
		output_HV = output_dma_O.read();
		
		if(output_HV.last == 1){
			break;
		}
		else{
			output_check[i] = output_HV.data;
	
		}
	}


	print_value(output_check);

	return 0;
}


void print_value(int* input){
	for(int i = 0; i < CHUNK_NUM*CHUNK_SIZE; i++){
		printf("%d , \t", input[i]);
	}
}
