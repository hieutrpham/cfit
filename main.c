#include <unistd.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fit_convert.h"
#include "fit_example.h"
#include <raylib.h>
#define FIT_CONVERT_TIME_RECORD
#define FIT_CONVERT_CHECK_CRC
#define LEFT_PAD 0
#define DATA_SIZE 1024
#define RECT_SIZE 2
#define CIR_SIZE 5
#if 0
#define WIDTH 1920
#define HEIGHT 1080
#define TEXT_SIZE 40
#else
#define WIDTH 800
#define HEIGHT 600
#define TEXT_SIZE 20
#endif

uint32_t hr_buffer[DATA_SIZE] = {0};
size_t count = 0;
uint32_t speed[DATA_SIZE] = {0};
float pace[DATA_SIZE] = {0};

float get_pixel(int data_min, int data_max, int pixel_min, int pixel_max, int value)
{
	float ratio = (float)(pixel_max - pixel_min)/(float)(data_max - data_min);
	return (float)pixel_min + (float)(value - data_min)*ratio;
}

int get_value(int data_min, int data_max, int pixel_min, int pixel_max, int pixel)
{
	float ratio = (float)(pixel_max - pixel_min)/(float)(data_max - data_min);
	int value = (pixel - pixel_min)/ratio + data_min;
	return value;
}

uint32_t find_min(uint32_t *buf, size_t buf_size)
{
	if (buf_size <= 0)
	{
		fprintf(stderr, "ERROR: empty hr_buffer\n");
		exit(1);
	}
	uint32_t min = buf[0];
	size_t i = 0;
	while (i < buf_size)
	{
		if (buf[i] < min)
			min = buf[i];
		i++;
	}
	return min;
}

uint32_t find_max(uint32_t *buf, size_t buf_size)
{
	if (buf_size <= 0)
	{
		fprintf(stderr, "ERROR: empty buffer\n");
		exit(1);
	}
	uint32_t max = buf[0];
	size_t i = 0;
	while (i < buf_size)
	{
		if (buf[i] > max)
			max = buf[i];
		i++;
	}
	return max;
}

/* @brief: convert speed ms to min/km. speed is in 1000*m/s
 */
float convert_min_km(float ms)
{
	return 1000*1000/(60*ms);
}

void usage()
{
	printf("Usage: ./main <fit file> [-whofit]\n");
	printf("    -w: screen width\n");
	printf("    -h: screen height\n");
	printf("    -o: circle size\n");
	printf("    -i: rectangle size \n");
	printf("    -t: text size \n");
	exit(1);
}

int main(int argc, char* argv[])
{
	int opt, width, height, rect_size, cir_size, text_size;
	char *file_name;

	if (argc < 2)
		usage();

	file_name = argv[1];
	width = WIDTH;
	height = HEIGHT;
	rect_size = RECT_SIZE;
	cir_size = CIR_SIZE;
	text_size = TEXT_SIZE;
	while ((opt = getopt(argc, argv, ":f:w:h:o:i:t:")) != -1)
	{
		switch (opt)
		{
			case 'f': file_name = optarg; break;
			case 'w': width = atoi(optarg); break;
			case 'h': height = atoi(optarg); break;
			case 'o': cir_size = atoi(optarg); break;
			case 'i': rect_size = atoi(optarg); break;
			case 't': text_size = atoi(optarg); break;
			default: usage();
		}
	}

	FILE *file;
	FIT_UINT8 buf[8];
	FIT_CONVERT_RETURN convert_return = FIT_CONVERT_CONTINUE;
	FIT_UINT32 buf_size;

	printf("Testing file conversion using %s file...\n", file_name);

	FitConvert_Init(FIT_TRUE);

	if((file = fopen(file_name, "rb")) == NULL)
	{
		printf("Error opening file %s.\n", file_name);
		return FIT_FALSE;
	}

	while(!feof(file) && (convert_return == FIT_CONVERT_CONTINUE))
	{
		for(buf_size=0;(buf_size < sizeof(buf)) && !feof(file); buf_size++)
			buf[buf_size] = (FIT_UINT8)getc(file);
		do
		{
			convert_return = FitConvert_Read(buf, buf_size);
			switch (convert_return)
			{
				case FIT_CONVERT_MESSAGE_AVAILABLE:
					{
						const FIT_UINT8 *mesg = FitConvert_GetMessageData();
						FIT_UINT16 mesg_num = FitConvert_GetMessageNumber();
						switch(mesg_num)
						{
							case FIT_MESG_NUM_RECORD:
								{
									const FIT_RECORD_MESG *record = (FIT_RECORD_MESG *) mesg;
									hr_buffer[count] = record->heart_rate;
									pace[count] = convert_min_km(record->speed);
									speed[count] = record->speed;
									count++;
									break;
								}
							default:
								break;
						}
						break;
					}

				default:
					break;
			}
		} while (convert_return == FIT_CONVERT_MESSAGE_AVAILABLE);
	}
	if (convert_return == FIT_CONVERT_ERROR)
	{
		printf("Error decoding file.\n");
		fclose(file);
		return 1;
	}
	if (convert_return == FIT_CONVERT_CONTINUE)
	{
		printf("Unexpected end of file.\n");
		fclose(file);
		return 1;
	}
	if (convert_return == FIT_CONVERT_DATA_TYPE_NOT_SUPPORTED)
	{
		printf("File is not FIT.\n");
		fclose(file);
		return 1;
	}
	if (convert_return == FIT_CONVERT_PROTOCOL_VERSION_NOT_SUPPORTED)
	{
		printf("Protocol version not supported.\n");
		fclose(file);
		return 1;
	}
	if (convert_return == FIT_CONVERT_END_OF_FILE)
		printf("File converted successfully.\n");
	fclose(file);

	// NOTE: parsing done

	uint32_t hr_max = find_max(hr_buffer, count);
	uint32_t hr_min = find_min(hr_buffer, count);
	uint32_t speed_max = find_max(speed, count);
	uint32_t speed_min = find_min(speed, count);
	InitWindow(width, height, "run");
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(BLACK);

		#if 1
		size_t i = 0;
		int index = get_value(0, count, 0, width, GetMouseX());
		DrawText(TextFormat("HR: %zu (bpm)\nPace: %.2f (min/km)", hr_buffer[index], pace[index]), 0, 0, text_size, GREEN);
		while (i < count)
		{
			DrawRectangle(
				LEFT_PAD + get_pixel(0, count, 0, width, i),
				height - (int)(fmin(get_pixel(speed_min, speed_max, 0, height, speed[i]), height)),
				rect_size,
				(int)(get_pixel(speed_min, speed_max, 0, height, speed[i]), height),
				YELLOW);
			DrawCircle(
				LEFT_PAD + get_pixel(0, count, 0, width, i),
				height - get_pixel(hr_min, hr_max, 0, height, hr_buffer[i]),
				cir_size,
				RED);
			i++;
		}
		#endif
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
