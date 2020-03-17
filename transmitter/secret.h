unsigned long buffToInt(byte * bytes) {

	unsigned long secret = long((unsigned long)(bytes[0]) << 24 |
								(unsigned long)(bytes[1]) << 16 |
								(unsigned long)(bytes[2]) << 8  |
								(unsigned long)(bytes[3]));
	
	return secret;

}

unsigned long generate_secret() {

	byte bytes[4];

	bytes[0] = getTrueRotateRandomByte() | 128; //Ensure highest bit is 1
	bytes[1] = getTrueRotateRandomByte();
	bytes[2] = getTrueRotateRandomByte();
	bytes[3] = getTrueRotateRandomByte();

	unsigned long secret = buffToInt(bytes);

	char buffer[80];
	sprintf(buffer, "Shared secret is: %lu", secret);
	Serial.println(buffer);

	return secret;
}