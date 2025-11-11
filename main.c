#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fit_convert.h"
#include "fit_example.h"
#include <raylib.h>
#define FIT_CONVERT_TIME_RECORD
#define FIT_CONVERT_CHECK_CRC
#define WIDTH 1920
#define HEIGHT 1080
#define LEFT_PAD 100
#define DATA_SIZE 1024*5

int hr_buffer[DATA_SIZE] = {0};
size_t hr_count = 0;
uint32_t timestamp[DATA_SIZE] = {0};
size_t ts_count = 0;
uint32_t distance[DATA_SIZE] = {0};
size_t distance_count = 0;

int linear_scale(int data_min, int data_max, int pixel_min, int pixel_max, int value)
{
	int ratio = (pixel_max - pixel_min)/(data_max - data_min);
	return pixel_min + (value - data_min)*ratio;
}

int find_min(int *buf, size_t buf_size)
{
	if (buf_size <= 0)
	{
		fprintf(stderr, "ERROR: empty hr_buffer\n");
		exit(1);
	}
	int min = buf[0];
	size_t i = 0;
	while (i < buf_size)
	{
		if (buf[i] < min)
			min = buf[i];
		i++;
	}
	return min;
}

int find_max(int *buf, size_t buf_size)
{
	if (buf_size <= 0)
	{
		fprintf(stderr, "ERROR: empty buffer\n");
		exit(1);
	}
	int max = buf[0];
	size_t i = 0;
	while (i < buf_size)
	{
		if (buf[i] > max)
			max = buf[i];
		i++;
	}
	return max;
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
									timestamp[ts_count++] = record->timestamp;
									distance[distance_count++] = record->distance;
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

	for (size_t i = 0; i < ts_count; i ++)
		printf("timestamp: %d\n", timestamp[i]);

	for (size_t i = 0; i < distance_count; i ++)
		printf("distance (m): %d\n", distance[i]/100);

	int hr_max = find_max(hr_buffer, hr_count);
	int hr_min = find_min(hr_buffer, hr_count);
	InitWindow(WIDTH, HEIGHT, "run");
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(BLACK);
		size_t i = 0;
		// NOTE: draw HR data
		while (i < hr_count)
		{
			DrawCircle(
				LEFT_PAD + linear_scale(0, hr_count, 0, WIDTH, i),
				HEIGHT - linear_scale(hr_min, hr_max, 0, HEIGHT, hr_buffer[i]),
				2,
				RED);
			i++;
		}
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
