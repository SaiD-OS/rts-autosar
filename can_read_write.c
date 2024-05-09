#include <Can.h>
#include <CanIf.h>
#include <stdio.h>
#include <string.h>
#include <tpl_posix_can_driver.h>

#include "tpl_memmap.h"
int main(void)
{
	// Statically list the configuration of each CAN controller used in the application
	static tpl_can_controller_config_t can_controllers_config = {
			&can_posix_driver_controller,
			{
				.CanControllerBaudRate = 250,
				.CanControllerBaudRateConfigID = 0,
				.CanControllerPropSeg = 0,
				.CanControllerSeg1 = 11,
				.CanControllerSeg2 = 4,
				.CanControllerSyncJumpWidth = 4,
				.use_fd_configuration = FALSE,
			}
		};
	
    static Can_ConfigType can_config_type =
	{
		&can_controllers_config,
		1
	};
	int ret;
	
	ret = Can_Init(&can_config_type);
	if (ret)
	{
		printf("[%s:%d] Error : Can_Init() failed (%d).\r\n", __func__, __LINE__, ret);
		return -1;
	}

	StartOS(OSDEFAULTAPPMODE);
	return 0;
}



TASK(CanReadWrite)
{
	Std_ReturnType ret;
	uint8 payload[64];
	Can_PduType can_pdu, *pointer_can_pdu;
	PduInfoType pdu_info;
	int i;

	printf("Transmitting a CAN 2.0 frame with standard ID...\r\n");

	//Test Payload
	strcpy((char *) payload, "This is longer string.");
	
	
	tpl_can_fill_pdu_info(&can_pdu, &pdu_info, 0x123 | TPL_CAN_ID_TYPE_STANDARD, payload, strlen((char *) payload));
	ret = CanIf_Transmit(0, &pdu_info);
	if (ret)
		printf("[%s:%d] Error : failed to transmit the frame (%d).\r\n", __func__, __LINE__, ret);
	printf("Transmission succeeded.\r\n");

	printf("Waiting for a CAN 2.0 frame with standard ID...\r\n");
	ret = CanIf_ReadRxPduData(0, &pdu_info);
	if (ret)
		printf("No frame is available.\r\n");
	else
	{
		printf("A frame has been received.\r\n");

		pointer_can_pdu = (Can_PduType *) pdu_info.SduDataPtr;
		printf("ID = 0x%X, flags = 0x%02X, length = %d, payload = ",
			pointer_can_pdu->id & TPL_CAN_ID_STANDARD_MASK,
			TPL_CAN_ID_TYPE_GET(pointer_can_pdu->id),
			pointer_can_pdu->length);
		for (i = 0; i < pointer_can_pdu->length; i++)
			printf("0x%02X ", pointer_can_pdu->sdu[i]);
		printf("\r\n");
	}

	TerminateTask();
}
