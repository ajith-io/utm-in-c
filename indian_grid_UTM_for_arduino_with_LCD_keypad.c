/**
 * @file indian_grid_UTM_for_arduino_with_LCD_keypad.c
 * @author Ajith
 * @brief 
 * @version 0.1
 * @date 2021-06-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <math.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

/**
 * @brief Ask ajith about the things
 * 1) reduce delay in printing menu (A, B, C, D)
 * 
 */

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
// initialize the library with the numbers of the interface pins


const byte numRows= 4; //number of rows on the keypad
const byte numCols= 4; //number of columns on the keypad
/*keymap defines the key pressed according to the row and columns just as appears on the keypad*/
char keymap[numRows][numCols]={
	{'1', '2', '3', 'A'},
	{'4', '5', '6', 'B'},
	{'7', '8', '9', 'C'},
	{'.', '0', '#', 'D'}
};
//Code that shows the the keypad connections to the arduino terminals
byte rowPins[numRows] = {5, 4, 3, 2}; //Rows 0 to 3
byte colPins[numCols]= {8, 9 ,10, 11}; //Columns 0 to 3
//initializes an instance of the Keypad class
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

String inputString;
double inputDouble;
char keypressed = NO_KEY;
String printString;
int cursor_pos_2nd_row = 0;


// constants 
// change this into constants
double K0 = 0.9996;
double E = 0.00669438;
double E2 = E * E;
double E3 = E2 * E;
double E_P2 = E / (1 - E);
double SQRT_E = sqrt(1 - E);
double _E = (1 - SQRT_E) / (1 + SQRT_E);
double _E2 = _E * _E;
double _E3 = _E2 * _E;
double _E4 = _E3 * _E;
double _E5 = _E4 * _E;
double M1 = (1 - E / 4 - 3 * E2 / 64 - 5 * E3 / 256);
double M2 = (3 * E / 8 + 3 * E2 / 32 + 45 * E3 / 1024);
double M3 = (15 * E2 / 256 + 45 * E3 / 1024);
double M4 = (35 * E3 / 3072);
double P2 = (3 / 2 * _E - 27 / 32 * _E3 + 269 / 512 * _E5);
double P3 = (21 / 16 * _E2 - 55 / 32 * _E4);
double P4 = (151 / 96 * _E3 - 417 / 128 * _E5);
double P5 = (1097 / 512 * _E4);
double R = 6378137;
char ZONE_LETTERS[] = "CDEFGHJKLMNPQRSTUVWXX" ;

char *errors[] = {
	"zone number error",
	"zone letter error", 
	"latitude error",
	"longitude error", 
	"easting error",
	"northing error",
	"not in boundary"
};

struct IG_coord{
		int zone_number;
		double easting;
		double northing;
		String zone_char;
}

struct UTM_coord{
	int zone_number;
	char zone_letter;
	double easting;
	double northing;
};

struct latloncoord{
	double latitude;
	double longitude;
};


void setup(){
	Serial.begin(9600);
	// initialize the LCD
	lcd.begin();

	// Turn on the blacklight and print a message.
	lcd.backlight();
	initprint();
}

void loop(){
	startprint();
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void initprint(){
	lcd.print("Welcome");
	delay(1000); 
	lcd.setCursor(0, 0);
	lcd.print("Press A for ");
	lcd.setCursor(0,1);
	lcd.print("lat long to IG"); 
	delay(2000);
	lcd.setCursor(0, 0);
	lcd.print("Press B for ");
	lcd.setCursor(0,1);
	lcd.print("IG to lat long");
	lcd.setCursor(1, 0);
	delay(2000);
	lcd.setCursor(0, 0);
	lcd.print("Press C for ");
	lcd.setCursor(0,1);
	lcd.print("lat long to utm"); 
	delay(2000);
	lcd.print("Press D for ");
	lcd.setCursor(0,1);
	lcd.print("utm to lat long");
	lcd.setCursor(1, 0);
	delay(2000);
	lcd.clear();
}

void startprint(){
	lcd.setCursor(0, 0);
	lcd.print("Enter A,B,C or D");
	lcd.setCursor(0, 1);
	while (1){ 
		keypressed = myKeypad.getKey();
		if (keypressed != NO_KEY){
			lcd.print(keypressed);
			delay(1000);
			lcd.clear();
			if (keypressed == 'A'){
				A_routine();
			}
			if (keypressed == 'B'){
				B_routine();
			}
			if (keypressed == 'C'){
				C_routine();
			}
			if (keypressed == 'D'){
				D_routine();
			}
			break; 
		}
	} 
}

void timer_and_check_D_is_pressed(){
	while (1){
		keypressed = myKeypad.getKey();
		if (keypressed == 'D'){
			break;
		}
		if (keypressed == '#'){
			resetFunc();  //call reset
		}
	}
}

void print_stuff_1st_row(){
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print(printString);
}

void print_stuff_2nd_row(int from_first = 0){
	if (from_first){
		cursor_pos_2nd_row = 0;
	}
	for (int i = 0; i < printString.length(); i++){
		lcd.setCursor(cursor_pos_2nd_row, 1);
		lcd.print(printString[i]);
		cursor_pos_2nd_row = cursor_pos_2nd_row + 1;
	}
}

void clear_2nd_row(){
	for (int i = 0; i < 16;i++){
		lcd.setCursor(i, 1);
		lcd.print(" ");
	}
}

double print_and_take(){
	print_stuff_1st_row();
	cursor_pos_2nd_row = 0;
	while (1){
		keypressed = myKeypad.getKey();
		if (keypressed != NO_KEY) {
			printString = keypressed;
			if ((keypressed >= '0' && keypressed <= '9') || (keypressed == '.')){     
				// only act on numeric keys and decimal point
				print_stuff_2nd_row();
				inputString += keypressed; // append new character to input string
				keypressed = NO_KEY;
			} 
			else if (keypressed == 'D') {
				if (inputString.length() > 0) {
					inputDouble = inputString.toDouble();// YOU GOT AN INTEGER NUMBER
					inputString = "";// clear input
					keypressed = NO_KEY;
					printString = inputDouble;
					print_stuff_2nd_row();
					return (inputDouble);
				}
			}
		 else if (keypressed == 'C') {
			clear_2nd_row();
			inputString = inputString.substring(0, inputString.length() - 1);
			printString = inputString;
			print_stuff_2nd_row(1);
		 }
		 else if (keypressed == '#'){
			resetFunc();  //call reset 
		 } 
		}
	} 
}

latloncoord take_latlong(){
	struct latloncoord latloninp;
	printString = "Enter lat";
	latloninp.latitude = print_and_take();
	printString = "Enter long";
	latloninp.longitude = print_and_take();
	return (latloninp);
}

UTM_coord take_UTM(){
	struct UTM_coord UTM_inp;
	int temp = 0;
	printString = "Enter zone num";
	temp = print_and_take();
	UTM_inp.zone_number = temp;
	printString = "Enter zone letter"; 
	temp = print_and_take();
	char t = 64 + temp;
	UTM_inp.zone_letter = t;
	printString = "Enter easting"; 
	UTM_inp.easting = print_and_take();
	printString = "Enter northing"; 
	UTM_inp.northing = print_and_take();
	return(UTM_inp);
}

void print_latlon(struct latloncoord latlonprint){
	printString = "Latitude";
	print_stuff_1st_row();
	printString = latlonprint.latitude;
	print_stuff_2nd_row(1);
	timer_and_check_D_is_pressed();
	printString = "Longitude";
	print_stuff_1st_row();
	printString = latlonprint.longitude;
	print_stuff_2nd_row(1);
	timer_and_check_D_is_pressed();
	lcd.clear();
}

void print_UTM(struct UTM_coord UTM_print){
	printString = "zone num";
	print_stuff_1st_row();
	printString = UTM_print.zone_number;
	print_stuff_2nd_row(1);
	timer_and_check_D_is_pressed();
	printString = "zone letter";
	print_stuff_1st_row();
	printString = UTM_print.zone_letter;
	print_stuff_2nd_row(1);
	timer_and_check_D_is_pressed();
	printString = "easting";
	print_stuff_1st_row();
	printString = UTM_print.easting;
	print_stuff_2nd_row(1);
	timer_and_check_D_is_pressed();
	printString = "northing";
	print_stuff_1st_row();
	printString = UTM_print.northing;
	print_stuff_2nd_row(1);
	timer_and_check_D_is_pressed();
	lcd.clear();
}

void A_routine(){
	struct latloncoord latlonA;
	struct IG_coord IG_A;
	latlonA = take_latlong();
	IG_A = from_latlon_to_IG(latlonA.latitude, latlonA.longitude);
	print_IG(IG_A);
}

void B_routine(){
	struct latloncoord latlonB;
	struct IG_coord IG_B;
	IG_B = take_IG();
	latlonB = to_latlon_from_IG(IG_B);
	print_latlon(latlonB);
}

void C_routine(){
	struct latloncoord latlonA;
	struct UTM_coord UTM_A;
	latlonA = take_latlong();
	UTM_A = from_latlon(latlonA.latitude, latlonA.longitude);
	print_UTM(UTM_A);
}

void D_routine(){
	struct latloncoord latlonB;
	struct UTM_coord UTM_B;
	UTM_B = take_UTM();
	latlonB = to_latlon(UTM_B);
	print_latlon(latlonB);
}



bool in_bounds(double x, double lower, double upper){
	return ((x >= lower) && (x <= upper));
}

double deg_to_rad(double degree){
	double radian = (degree * 71.0) / 4068.0;
	return (radian);
}

double rad_to_deg(double radian){
	double degree = (radian * 4068.0) / 71.0;
	return (degree);
	}

void check_valid_zone(int zone_number,char zone_letter){
	/*This function checks whether the zone_number and zone_letter are valid
		Parameters
		----------
		easting: int
			The zone_number of the UTM coordinates 
		northing: char
			The zone_letter of the UTM coordinates this must be uppercase
		Returns
		-------
		None
		It just prints the error
	*/
	if (!((zone_number >= 1) && (zone_number <= 60))){
		//zone number out of range (must be between 1 and 60)
		printString = errors[0];
		print_stuff_1st_row();
		delay(2000);
		resetFunc();
	}

	if (zone_letter){
		if (!(((zone_number >= 'C') && (zone_number <= 'X')) || ((zone_letter != 'I') && (zone_letter != 'O')))){
			// zone letter out of range (must be between C and X
			printString = errors[1];
			print_stuff_1st_row();
			delay(2000);
			resetFunc();
		}
	}
}

int latlon_to_zone_number(double latitude, double longitude){
	/*This function checks whether the zone_number and zone_letter are valid
		Parameters
		----------
		latitude: double
			The latitude 
		longitude: double
			The longitude
		Returns
		-------
		zone_number : int 
			The zone_number of the respective UTM coordinate after conversion
	*/

	if (((latitude- 56)*(latitude- 64) <= 0) && ((latitude- 3)*(latitude - 12) <= 0))
		return (32);

	if (((latitude- 72)*(latitude- 84) <= 0) && (longitude >= 0)){
		if (longitude < 9)
			return (31);
		else if (longitude < 21)
			return (33);
		else if (longitude < 33)
			return (35);
		else if (longitude < 42)
			return (37);
	}

	return (int((longitude + 180) / 6) + 1);
}

char latitude_to_zone_letter(double latitude){
	/*This function checks whether the zone_number and zone_letter are valid
		Parameters
		----------
		latitude: double
			The latitude 
		Returns
		-------
		zone_letter : char 
			The zone_letter of the respective UTM coordinate after conversion
	*/
	return (ZONE_LETTERS[int(latitude + 80) >> 3]);
}

double zone_number_to_central_longitude(int zone_number){
	/*This function returns the central longitude of a zone
		Parameters
		----------
		zone_number : int
			The zone_number of the UTM coordinate 
		Returns
		-------
		central longitude :
			The central longitude of the given zone
	*/
	double rt = ((zone_number - 1) * 6 - 180 + 3); 
	return (rt);
}

double mod_angle(double value){
	/* Returns angle in radians to be between -pi and pi */
	if (value >0)
		 return (fmod(value + M_PI, 2.0*M_PI) - M_PI);
	else
		return (fmod(value - M_PI, 2.0*M_PI ) + M_PI);
}

bool negative(double x){
	return (x < 0);
}


UTM_coord from_latlon(double latitude, double longitude){
	/*This function converts Latitude and Longitude to UTM coordinate
		Parameters
		----------
		latitude: double
			Latitude between 80 deg S and 84 deg N, e.g. (-80.0 to 84.0)
		longitude: double 
			Longitude between 180 deg W and 180 deg E, e.g. (-180.0 to 180.0)
		Returns
		-------
		Returns the struct UTM_conv which is derieved from UTM_coord and has the following things :
		zone_number: int
			Zone number is represented by global map numbers of a UTM zone
			numbers map. 
		zone_letter: char
			Zone letter is represented by a string value. 
		easting: double
			Easting value of UTM coordinates
		northing: double
			Northing value of UTM coordinates
	*/ 
	if (!(in_bounds(latitude, -80, 84))){
		printString = errors[2];
		print_stuff_1st_row();
		delay(2000);
		resetFunc();
	}
	if (!(in_bounds(longitude, -180, 180))){
		printString = errors[3];
		print_stuff_1st_row();
		delay(2000);
		resetFunc();
	}

	double lat_rad = deg_to_rad(latitude);
	double lat_sin = sin(lat_rad);
	double lat_cos = cos(lat_rad);
	double lat_tan = lat_sin / lat_cos;
	double lat_tan2 = lat_tan * lat_tan;
	double lat_tan4 = lat_tan2 * lat_tan2;

	struct UTM_coord UTM_conv;

	UTM_conv.zone_number = latlon_to_zone_number(latitude, longitude);

	UTM_conv.zone_letter = latitude_to_zone_letter(latitude);

	double lon_rad = deg_to_rad(longitude);
	double central_lon = zone_number_to_central_longitude(UTM_conv.zone_number);
	double central_lon_rad = deg_to_rad(central_lon);

	double n = (R / (sqrt(1 - E * pow(lat_sin, 2.0))));
	double c = (E_P2 * pow(lat_cos,2));

	double a = (lat_cos * mod_angle(lon_rad - central_lon_rad));
	double a2 = a * a;
	double a3 = a2 * a;
	double a4 = a3 * a;
	double a5 = a4 * a;
	double a6 = a5 * a;

	double m = R * ((M1 * lat_rad) - (M2 * sin(2 * lat_rad)) + (M3 * sin(4 * lat_rad)) - (M4 * sin(6 * lat_rad)));

	UTM_conv.easting = K0 * n * (a + (a3 / 6 * (1 - lat_tan2 + c)) + (a5 / 120 * (5 - 18 * lat_tan2 + lat_tan4 + 72 * c - 58 * E_P2))) + 500000;

	UTM_conv.northing = K0 * (m + n * lat_tan * (a2 / 2 + (a4 / 24 * (5 - lat_tan2 + 9 * c + 4 * pow(c, 2))) + (a6 / 720 * (61 - 58 * lat_tan2 + lat_tan4 + 600 * c - 330 * E_P2))));

	if (negative(latitude))
		UTM_conv.northing += 10000000;

	return (UTM_conv);

}
latloncoord to_latlon(struct UTM_coord UTM_conv){
		/*This function converts UTM coordinates to Latitude and Longitude
				Parameters
				----------
				zone_number: int
						Zone number is represented with global map numbers of a UTM zone
						numbers map. For more information see utmzones [1]_
				zone_letter: char
						Zone letter can be represented as string values.
				easting: double
						Easting value of UTM coordinates
				northing: double
						Northing value of UTM coordinates
				Returns
				-------
				latitude: double
						Latitude between 80 deg S and 84 deg N, e.g. (-80.0 to 84.0)
				longitude: double
						Longitude between 180 deg W and 180 deg E, e.g. (-180.0 to 180.0).
		*/
		if (!(in_bounds(UTM_conv.easting, 100000, 1000000))){
			printString = errors[4];
			print_stuff_1st_row();
			delay(2000);
			resetFunc();
		}
		if (!(in_bounds(UTM_conv.northing, 0, 10000000))){
			printString = errors[5];
			print_stuff_1st_row();
			delay(2000);
			resetFunc();
		}
		check_valid_zone(UTM_conv.zone_number, UTM_conv.zone_letter);

		double x = UTM_conv.easting - 500000;
		double y = UTM_conv.northing;
		double m = y / K0;
		double mu = m / (R * M1);

		double p_rad = (mu + (P2 * sin(2 * mu)) + (P3 * sin(4 * mu)) + (P4 * sin(6 * mu)) + (P5 * sin(8 * mu)));

		double p_sin = sin(p_rad); 
		double p_sin2 = p_sin * p_sin; 

		double p_cos = cos(p_rad);

		double p_tan = p_sin / p_cos; 
		double p_tan2 = p_tan * p_tan;
		double p_tan4 = p_tan2 * p_tan2; 

		double ep_sin = 1 - E * p_sin2;
		double ep_sin_sqrt = sqrt(1 - E * p_sin2);

		double n = R / ep_sin_sqrt;
		double r = (1 - E) / ep_sin;

		double c = E_P2 * pow(p_cos, 2);
		double c2 = c * c;

		double d = x / (n * K0);
		double d2 = d * d;
		double d3 = d2 * d;
		double d4 = d3 * d;
		double d5 = d4 * d;
		double d6 = d5 * d;

		struct latloncoord latlonconv;
		latlonconv.latitude = (p_rad - (p_tan / r) * ((d2 / 2) - (d4 / 24 * (5 + 3 * p_tan2 + 10 * c - 4 * c2 - 9 * E_P2))) + (d6 / 720 * (61 + 90 * p_tan2 + 298 * c + 45 * p_tan4 - 252 * E_P2 - 3 * c2)));

		latlonconv.longitude = (d - (d3 / 6 * (1 + 2 * p_tan2 + c)) + (d5 / 120 * (5 - 2 * c + 28 * p_tan2 - 3 * c2 + 8 * E_P2 + 24 * p_tan4)) / p_cos);

		latlonconv.longitude = mod_angle(latlonconv.longitude + deg_to_rad(zone_number_to_central_longitude(UTM_conv.zone_number)));
		latlonconv.latitude = rad_to_deg(latlonconv.latitude);
		latlonconv.longitude = rad_to_deg(latlonconv.longitude);
		return (latlonconv);
}


int in_zone(latitude, longitude, lat1, lat2, long1, long2){
		int lat_cond = (latitude >= lat1) && (latitude <= lat2);
		int long_cond = (longitude >= long1) && (longitude <= long2);
		if ((lat_cond) && (long_cond)){
				return (1);
		}
		else{
				return (0);
		}
}


int find_zone(latitude, longitude){
		if (in_zone(latitude, longitude, 28, 35.51, 70.34, 81.64)){
				return (1)
		}
		else if (in_zone(latitude, longitude, 21, 28.01, 61.59, 82.01)){
				return (2)
		}
		else if (in_zone(latitude, longitude, 21, 29.47, 82, 101.17)){
				return (3)
		} 
		else if (in_zone(latitude, longitude, 15, 21.01, 70.14, 87.15)){
				return (4)
		} 
		else if (in_zone(latitude, longitude, 8.02, 15.01, 73.94, 80.41)){
				return (5)
		}
		else{
				return (0)
		}
}

IG_coord from_latlon_to_IG (double latitude, double longitude){
	/*This function converts Latitude and Longitude to UTM coordinate
		Parameters
		----------
		latitude: double
			Latitude between 80 deg S and 84 deg N, e.g. (-80.0 to 84.0)
		longitude: double 
			Longitude between 180 deg W and 180 deg E, e.g. (-180.0 to 180.0)
		Returns
		-------
		Returns the struct IG_conv which is derieved from IG_coord and has the following things :
		zone_number: int
			Zone number is represented by this and the following boundaries
				1 represents 1A = 28, 35.51, 70.34, 81.64   https://epsg.org/conversion_18231/India-zone-I-1975-metres.html
				2 represents 2A = 21, 28.01, 61.59, 82.01   https://epsg.org/crs_24372/Kalianpur-1880-India-zone-IIa.html?sessionkey=yau30xjcaz
				3 represents 2B = 21, 29.47, 82, 101.17     https://epsg.org/crs_24382/Kalianpur-1880-India-zone-IIb.html?sessionkey=yau30xjcaz
				4 represents 3A = 15, 21.01, 70.14, 87.15   https://epsg.org/crs_24373/Kalianpur-1880-India-zone-IIIa.html?sessionkey=yau30xjcaz
				5 represents 4A = 8.02, 15.01, 73.94, 80.41 https://epsg.org/crs_24374/Kalianpur-1880-India-zone-IVa.html?sessionkey=yau30xjcaz
				there is no zone 0, 1B, 3B, 4B
		easting: double
			Easting value of IG coordinates
		northing: double
			Northing value of IG coordinates
	*/ 
 if (!(in_bounds(latitude, -80, 84))){
		printString = errors[2];
		print_stuff_1st_row();
		delay(2000);
		resetFunc();
	}
	if (!(in_bounds(longitude, -180, 180))){
		printString = errors[3];
		print_stuff_1st_row();
		delay(2000);
		resetFunc();
	}
	double latod = 0.00;
	double longod = 0.00;
	
	struct IG_coord IG_conv;

	IG_conv.zone_number = find_zone(latitude, longitude);

	if (0 == IG_conv.zone_number){
		printString = errors[6];
		print_stuff_1st_row();
		delay(2000);
		resetFunc();
	}
	switch (IG_conv.zone_number){
		// https://epsg.org/conversion_18231/India-zone-I-1975-metres.html
		case 1:
				latod = 32.50;
				longod = 68;
				IG_conv.zone_char = "1A";
				break;

		case 2:
				latod = 26;
				longod = 74; 
				IG_conv.zone_char = "2A";
				break;
		
		case 3:
				latod = 26;
				longod = 90;
				IG_conv.zone_char = "2B";
				break;
		
		case 4:
				latod = 19;
				longod = 80;
				IG_conv.zone_char = "3A";
				break;
		case 5:
				latod = 12;
				longod = 80;
				IG_conv.zone_char = "4A";
				break;
		}

	double lato = deg_to_rad(latod);
	double longo = deg_to_rad(longod);
	double latd = latitude;
	double longd = longitude;
	double lat_rad = deg_to_rad(latd);
	double long_rad = deg_to_rad(longd);
	double e = 0.081472981;
	double e_2 = 0.00676866;
	double FE = 2743195.5;
	double FN = 914398.5;
	double a = 6377301.243;
	double k = 0.99878641;
	double mo = (cos(lato) / pow(1 - ((e_2)*pow(sin(lato), 2)), 0.5);
	double n= sin(lato);
	double to = (tan(M_PI / 4 - lato / 2) / pow((1-(e*sin(lato)))/(1 + (e*sin(lato)))), (e/2));
	double F = mo/(n*pow(to, n));
	double ro = a*F*pow(to, n)*k;
	double t = (tan((M_PI/4)-(lat_rad/2)) / pow((1-(e*sin(lat_rad)))/(1+(e*sin(lat_rad))), (e/2)));
	double r = a*F*pow(t, n)*k;
	double theta = n*(long_rad - longo);
	double E = FE + (r * sin(theta));
	double N = FN + ro - (r * cos(theta));
	IG_conv.easting = E;
	IG_conv.northing = N;
	return (IG_conv);
}


void print_IG(struct IG_coord IG_print){
	printString = "zone num";
	print_stuff_1st_row();
	printString = IG_print.zone_char;
	print_stuff_2nd_row(1);
	timer_and_check_D_is_pressed();
	printString = "easting";
	print_stuff_1st_row();
	printString = IG_print.easting;
	print_stuff_2nd_row(1);
	timer_and_check_D_is_pressed();
	printString = "northing";
	print_stuff_1st_row();
	printString = IG_print.northing;
	print_stuff_2nd_row(1);
	timer_and_check_D_is_pressed();
	lcd.clear();
}


IG_coord take_IG(){
	struct IG_coord IG_inp;
	int temp = 0;
	printString = "Enter zone num";
	IG_inp.zone_char = print_and_take_zone_num_IG()
	switch (IG_inp.zone_char)){
		case ("1A"):
			temp = 1;
			break;
		case ("2A"):
			temp = 2;
			break;
		case ("2B"):
			temp = 3;
			break;
		case ("3A"):
			temp = 4;
			break;
		case ("4A"):
			temp = 5;
			break;
		default:
			temp = 0;
			break;
	}
	IG_inp.zone_number = temp;
	printString = "Enter easting"; 
	IG_inp.easting = print_and_take();
	printString = "Enter northing"; 
	IG_inp.northing = print_and_take();
	return(IG_inp);
}

latloncoord to_latlon_from_IG(struct IG_coord IG_conv){
	from math import atan2, atan, pow, sqrt, pi, degrees, exp, sin
	double E = IG_conv.easting;
	double N = IG_conv.northing;
	double FE = 2743196.4;
	double FN = 914398.8;
	double e = 0.081472981;
	double e_2 = 0.00676866;
	double FE = 2743195.5;
	double FN = 914398.5;
	double a = 6377301.243;
	double k = 0.99878641;
	double latod = 0.00;
	double longod = 0.00;
	if (0 == IG_conv.zone_number){
		printString = errors[6];
		print_stuff_1st_row();
		delay(2000);
		resetFunc();
	}
	switch (IG_conv.zone_number){
		// https://epsg.org/conversion_18231/India-zone-I-1975-metres.html
		case 1:
				latod = 32.50;
				longod = 68;
				break;

		case 2:
				latod = 26;
				longod = 74; 
				break;
		
		case 3:
				latod = 26;
				longod = 90;
				break;
		
		case 4:
				latod = 19;
				longod = 80;
				break;
		case 5:
				latod = 12;
				longod = 80;
				break;
		}

	double lato = deg_to_rad(latod);
	double longo = deg_to_rad(longod);
	double n = sin(lato);
	double F = mo/(n*pow(to, n));
	double ro = a*F*pow(to, n)*k;
	double r_dash = sqrt(pow(E - FE, 2) + pow(ro - (N - FN), 2));
	// assigning the sign of r_dash based on the sign of n
	if (n >= 0){
		if !(r_dash >= 0)
			r_dash = (-1.0) * (r_dash);
	}
	else{
		if (r_dash >= 0)
			r_dash = (-1.0) * (r_dash);
	}
	// atan2 is a function 
	// Arc tangent of two numbers, or four-quadrant inverse tangent. 
	// ATAN2(y,x) returns the arc tangent of the two numbers x and y. 
	// It is similar to calculating the arc tangent of y / x, except 
	// that the signs of both arguments are used to determine the 
	// quadrant of the result.
	// The result is an angle expressed in radians.
	if (n >= 0){
		theta_dash = atan2((E - FE) , (ro - (N - FN)));
	} 
	else{
		theta_dash = atan2(-(E - FE) , -(ro - (N - FN)));
	}
	double lon = rad_to_deg((theta_dash/n) + longo);
	double t_dash = pow(r_dash/(a*k*F) , 1/n);
	// write based on n sign assign theta
	double lat = ((pi/2) - (2 * atan(t_dash)));
	double inter = 0;
	// iterating through the for loop again and again 
	// for 10 times
	for(int i = 0; i < 10; i++){
		inter = ((1 - (e * sin(lat)))/(1 + (e* sin(lat))));
		lat = (pi/2) - (2*atan(t_dash* pow(inter,(e/2))));
	}
	lat = rad_to_deg(lat);
	struct latloncoord latlonconv;
	latlonconv.latitude = lat;
	latlonconv.longitude = lon;
	return (latlonconv);
}

String print_and_take_zone_num_IG(){
	String resString;
	print_stuff_1st_row();
	cursor_pos_2nd_row = 0;
	while (1){
		keypressed = myKeypad.getKey();
		if (keypressed != NO_KEY) {
			printString = keypressed;
			if ((keypressed >= '0' && keypressed <= '9') || (keypressed == '.')){     
				// only act on numeric keys and decimal point
				print_stuff_2nd_row();
				inputString += keypressed; // append new character to input string
				keypressed = NO_KEY;
			}
			else if ((keypressed == 'A') || (keypressed == 'B')){
				print_stuff_2nd_row();
				inputString += keypressed; // append new character to input string
				keypressed = NO_KEY;
			} 
			else if (keypressed == 'D') {
				if (inputString.length() > 0) {
					resString = inputString;
					inputString = "";// clear input
					keypressed = NO_KEY;
					printString = inputString;
					print_stuff_2nd_row();
					return (resString);
				}
			}
		 else if (keypressed == 'C') {
			clear_2nd_row();
			inputString = inputString.substring(0, inputString.length() - 1);
			printString = inputString;
			print_stuff_2nd_row(1);
		 }
		 else if (keypressed == '#'){
			resetFunc();  //call reset 
		 } 
		}
	} 
}

