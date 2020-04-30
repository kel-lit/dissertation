typedef unsigned long ulong;
typedef unsigned int uint;

byte sbox(byte segment) {

	byte substitutions[16] = {0xf, 0xc, 0x2, 0x7, 0x9, 0x0, 
	0x5, 0xa, 0x1, 0xb, 0xe, 0x8, 0x6, 0xd, 0x3, 0x4};

	byte new_segment;

	new_segment += substitutions[(segment & 0x0f)];
	new_segment += substitutions[((segment >> 4) & 0x0f)] << 4;

	return new_segment;
}

byte inv_sbox(byte segment) {

	byte substitutions[16] = {0x5, 0x8, 0x2, 0xe, 0xf, 0x6, 
	0xc, 0x3, 0xb, 0x4, 0x7, 0x9, 0x1, 0xd, 0xa, 0x0};

	byte inv_segment;

	inv_segment += substitutions[(segment & 0x0f)];
	inv_segment += substitutions[((segment >> 4) & 0x0f)] << 4;

	return inv_segment;
}

void encrypt(byte *payload, ulong secret) {
	//Crude encryption with sbox and XORing
	for (int i=0; i < 7; i++){ // 7 Rounds

		for (int j=0; j < 4; j++) {
			payload[j] ^= (byte)(secret >> (j * 8));
			payload[j] = sbox(payload[j]);
		}

		byte tmp[2] = {payload[0], payload[1]};

		payload[0] = payload[2];
		payload[1] = payload[3];
		payload[2] = tmp[0];
		payload[3] = tmp[1];

	}	

    return;

}

void decrypt(byte *cipher, ulong secret) {
	//Crude decryption with sbox and XORing
	for (int i=0; i < 7; i++){ // 7 Rounds

		byte tmp[2] = {cipher[0], cipher[1]};

		cipher[0] = cipher[2];
		cipher[1] = cipher[3];
		cipher[2] = tmp[0];
		cipher[3] = tmp[1];

		for (int j=0; j < 4; j++) {
			cipher[j] = inv_sbox(cipher[j]);
			cipher[j] ^= (byte)(secret >> (j * 8));
		}
	}	

	return;

}

