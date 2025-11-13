#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fit_convert.h"
#include "fit_example.h"
#include <raylib.h>
#define FIT_CONVERT_TIME_RECORD
#define FIT_CONVERT_CHECK_CRC
#define LEFT_PAD 100
#define DATA_SIZE 1024*5
#if 1
#define WIDTH 1920
#define HEIGHT 1080
#else
#define WIDTH 800
#define HEIGHT 600
#endif

uint32_t hr_buffer[DATA_SIZE] = {0};
size_t hr_count = 0;
uint32_t speed[DATA_SIZE] = {0};
size_t speed_count = 0;

float linear_scale(int data_min, int data_max, int pixel_min, int pixel_max, int value)
{
	float ratio = (pixel_max - pixel_min)/(data_max - data_min);
	return pixel_min + (value - data_min)*ratio;
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

int main(int argc, char* argv[])
{
	FILE *file;
	FIT_UINT8 buf[8];
	FIT_CONVERT_RETURN convert_return = FIT_CONVERT_CONTINUE;
	FIT_UINT32 buf_size;

	if (argc < 2)
	{
		printf("usage: ./main <fit file>");
		return FIT_FALSE;
	}

	printf("Testing file conversion using %s file...\n", argv[1]);

	FitConvert_Init(FIT_TRUE);

	if((file = fopen(argv[1], "rb")) == NULL)
	{
		printf("Error opening file %s.\n", argv[1]);
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
									hr_buffer[hr_count++] = record->heart_rate;
									speed[speed_count++] = convert_min_km(record->speed);
									// speed[speed_count++] = record->speed;
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

	uint32_t hr_max = find_max(hr_buffer, hr_count);
	uint32_t hr_min = find_min(hr_buffer, hr_count);
	uint32_t speed_max = find_max(speed, speed_count);
	uint32_t speed_min = find_min(speed, speed_count);
	printf("speed max: %u, speed min: %u\n", speed_max, speed_min);
	InitWindow(WIDTH, HEIGHT, "run");
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(BLACK);
		size_t i = 0;
		while (i < hr_count)
		{
			DrawCircle(
				LEFT_PAD + linear_scale(0, hr_count, 0, WIDTH, i),
				HEIGHT - linear_scale(hr_min, hr_max, 0, HEIGHT, hr_buffer[i]),
				2,
				RED
				);
			// printf("INFO: %f\n", fmin(linear_scale(0, 7, 0, HEIGHT, speed[i]), HEIGHT));
			// printf("INFO: %u\n", speed[i]);
			// printf("%f\n", linear_scale(speed_min, speed_max, 0, HEIGHT, speed[i]));
			DrawRectangleLines(
				LEFT_PAD + linear_scale(0, speed_count, 0, WIDTH, i),
				HEIGHT,
				2,
				// (int)(fmin(linear_scale(speed_min, speed_max, 0, HEIGHT, speed[i]), HEIGHT)),
				(int)(fmin(linear_scale(0, 7, 0, HEIGHT, speed[i]), HEIGHT)) - HEIGHT,
				YELLOW);
			i++;
		}
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
