#include "data_tokenize.h"

using namespace std;

void Load_Lookup_Key(hls::stream<DMA_DATA> &input_lookup_char, hls::stream<DMA_DATA> &check_DMA_in, FPGA_DATA lookup_key[NUM_TOKEN]){
	
	DMA_DATA in_val;

	KEY0:for(int i = 0; i < NUM_TOKEN; i++){
	#pragma HLS PIPELINE
		in_val = input_lookup_char.read();
		lookup_key[i] = in_val.data;

		//check input
		// if(i == NUM_TOKEN-1){
		// 	in_val.last = 1;
		// }
		// else{
		// 	in_val.last = 0;
		// }
		check_DMA_in.write(in_val);
		if(in_val.last == 1){
			break;
		}
	}
} 

void Load_Lookup_Data(hls::stream<HV_DATA_I>& input_lookup_data, hls::stream<HV_DATA_I> &check_HV_in, HV_VEC lookup_value[CHUNK_NUM][CHUNK_SIZE]){

	HV_DATA_I in_data;

	//#pragma HLS PIPELINE
	DATA0:for(int i = 0; i < CHUNK_NUM; i++){
		DATA1:for(int j = 0; j < CHUNK_SIZE; j++){
			in_data = input_lookup_data.read();
			lookup_value[i][j] = in_data.data;

			//check input
			// if(i == CHUNK_NUM-1 && j == CHUNK_SIZE-1){
			// 	in_data.last = 1;
			// }
			// else{
			// 	in_data.last = 0;
			// }
			check_HV_in.write(in_data);
			if(in_data.last == 1){
				break;
			}
		}			
	}
} 

void Load_IFM(hls::stream<DMA_DATA>& input_dma_char, FPGA_DATA input[], int IFM_len){

	DMA_DATA in_val;
	IFM1:for(int i = 0; i < IFM_len; i++){
	#pragma HLS PIPELINE
		in_val = input_dma_char.read();
		input[i] = in_val.data;
	}
}

void ENCODING_HV(FPGA_DATA *input, 
				 FPGA_DATA lookup_key[NUM_TOKEN], 
				 HV_VEC lookup_value[CHUNK_NUM][CHUNK_SIZE],
				 int result_HV[CHUNK_NUM][CHUNK_SIZE],
				 int IFM_len){
	//Check input 

	EN_HV0:for(int i = 0; i < IFM_len; i++){
		EN_HV1:for(int j = 0; j < NUM_TOKEN; j++){
			if(input[i] == lookup_key[j]){
				int position = j;
				EN_HV2:for(int k = 0; k < CHUNK_NUM; k++){
	//#pragma HLS PIPELINE II=1
					position = position%CHUNK_NUM; 
					EN_HV3:for(int l = 0; l < CHUNK_SIZE; l++){
	#pragma HLS ARRAY_RESHAPE variable=lookup_value complete dim=2
    #pragma HLS ARRAY_RESHAPE variable=result_HV complete dim=2
						if(lookup_value[position][l] == 1)
							result_HV[k][l] = result_HV[k][l] + 1;
						else
							result_HV[k][l] = result_HV[k][l] - 1;
					}
					position = position+1;
				}
			}
		}
	}

	// for(int i=0; i<CHUNK_NUM; i++){
	// 	for(int j=0; j<CHUNK_SIZE; j++){
	// 		cout << result_HV[i][j] << ", \t";
	// 	}
	// 	cout << endl;
	// }
}


void OFM_STORE(hls::stream<HV_DATA_O>& output_dma_O, int result_HV[CHUNK_NUM][CHUNK_SIZE]){
	HV_DATA_O OutStream;

	OFM0:for(int i = 0; i < CHUNK_NUM; i++){
		OFM1:for(int j = 0; j < CHUNK_SIZE; j++){
			OutStream.data = result_HV[i][j];
			// if(i == CHUNK_NUM-1 && j == CHUNK_SIZE-1){
			// 	OutStream.last = 1;
			// }
			// else{
			// 	OutStream.last = 0;
			// }
			output_dma_O.write(OutStream);
			if(i == CHUNK_NUM-1 && j == CHUNK_SIZE-1){
				break;
			}
		}
	}

	cout << endl;
}


void CREATE_HV(hls::stream<DMA_DATA> &input_dma_char, 
					hls::stream<DMA_DATA> &input_lookup_char, 
					hls::stream<HV_DATA_I> &input_lookup_data, 
					hls::stream<HV_DATA_O> &output_dma_O,
					hls::stream<DMA_DATA> &check_DMA_in,
					hls::stream<HV_DATA_I> &check_HV_in,
					int state,
					int IFM_len){

	#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS
	#pragma HLS INTERFACE s_axilite port=state bundle=CRTL_BUS
	#pragma HLS INTERFACE s_axilite port=IFM_len bundle=CRTL_BUS

	#pragma HLS INTERFACE axis port=input_dma_char
	#pragma HLS INTERFACE axis port=input_lookup_data
	#pragma HLS INTERFACE axis port=input_lookup_char 
	#pragma HLS INTERFACE axis port=output_dma_O 

	/////////Check input value/////
	#pragma HLS INTERFACE axis port=check_DMA_in
	#pragma HLS INTERFACE axis port=check_HV_in 
	///////////////////////////////

	static FPGA_DATA input[in_len];
	#pragma HLS RESOURCE variable=input core=RAM_1P_BRAM
	#pragma HLS ARRAY_PARTITION variable=input complete dim=1

	static FPGA_DATA lookup_key[NUM_TOKEN];
	#pragma HLS RESOURCE variable=lookup_key core=RAM_1P_BRAM
	#pragma HLS ARRAY_PARTITION variable=lookup_key complete dim=1

	static HV_VEC lookup_value[CHUNK_NUM][CHUNK_SIZE];
	#pragma HLS RESOURCE variable=lookup_value core=RAM_1P_BRAM
	#pragma HLS ARRAY_PARTITION variable=lookup_value complete dim=1

	static int result_HV[CHUNK_NUM][CHUNK_SIZE];
	#pragma HLS RESOURCE variable=result_HV core=RAM_1P_BRAM
	#pragma HLS ARRAY_PARTITION variable=result_HV complete dim=1
	#pragma HLS ARRAY_PARTITION variable=result_HV complete dim=2

	if(state == 1){
		Load_Lookup_Key(input_lookup_char, check_DMA_in, lookup_key);
		Load_Lookup_Data(input_lookup_data, check_HV_in, lookup_value);
		for(int i = 0; i < CHUNK_NUM; i++){
			for(int j = 0; j < CHUNK_SIZE; j++){
			result_HV[i][j] = 1; 
			}
		}
	}

	else if(state == 2){
		Load_IFM(input_dma_char, input, IFM_len);
		ENCODING_HV(input, lookup_key, lookup_value, result_HV, IFM_len);
	}
	else if(state == 3){
		OFM_STORE(output_dma_O, result_HV);
	}	

}
