#include <stdio.h>
#include <string.h>
#include "fit_convert.h"
#include "fit_example.h"
#include <raylib.h>
#define FIT_CONVERT_TIME_RECORD
#define FIT_CONVERT_CHECK_CRC
#define WIDTH 800
#define HEIGHT 600

int buffer[1024*1024*5] = {0};
size_t count = 0;

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
						// printf("mesg num: %d\n", mesg_num);
						switch(mesg_num)
						{
							case FIT_MESG_NUM_RECORD:
								{
									const FIT_RECORD_MESG *record = (FIT_RECORD_MESG *) mesg;
									buffer[count++] = record->heart_rate;
									// printf("Record: timestamp=%u\n", record->timestamp);
									// printf("Record: heart_rate=%d\n", record->heart_rate);
									// printf("Record: distance=%u\n", record->distance);
									// printf("Record: speed=%u\n", record->speed);
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

	InitWindow(800, 600, "run");
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(BLACK);
		int i = 0;
		while (buffer[i])
		{
			DrawCircle(i + 50, HEIGHT - buffer[i] - 200, 1, RED);
			i++;
		}
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
