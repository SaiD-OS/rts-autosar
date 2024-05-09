#include <Can.h>
#include <string.h>
#include <unistd.h>

#include <tpl_posix_can_driver.h>
#include <tpl_os.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#define CAN_FRAME_ID 0x011
#define CAN_PAYLOAD_SIZE 8
#define CAN_FD_PAYLOAD_SIZE 64
#define CAN_PT_HEAD1 0x53
#define CAN_PT_HEAD2 0x54
#define CAN_PT_TAIL1 0x67
#define CAN_PT_TAIL2 0x98

static int can_posix_driver_init(struct tpl_can_controller_config_t *config);
static int can_posix_driver_set_baudrate(struct tpl_can_controller_t *ctrl, CanControllerBaudrateConfig *baud_rate_config);
static Std_ReturnType can_posix_driver_transmit(struct tpl_can_controller_t *ctrl, const Can_PduType *pdu_info);
static Std_ReturnType can_posix_driver_receive(struct tpl_can_controller_t *ctrl, Can_PduType *pdu_info);
static int can_posix_driver_is_data_available(struct tpl_can_controller_t *ctrl);

struct can_posix_driver_priv
{
	int is_can_fd_enabled;
};

static struct can_posix_driver_priv can_posix_driver_controller_priv;

tpl_can_controller_t can_posix_driver_controller =
{
	0xFFFFFFFF,
	can_posix_driver_init,
	can_posix_driver_set_baudrate,
	can_posix_driver_transmit,
	can_posix_driver_receive,
	can_posix_driver_is_data_available,
	&can_posix_driver_controller_priv
};

static int can_posix_driver_init(struct tpl_can_controller_config_t *config)
{
	struct can_posix_driver_priv *priv = config->controller->priv;

	// Determine the CAN protocol version
	if (config->baud_rate_config.use_fd_configuration)
		priv->is_can_fd_enabled = 1;
	else
		priv->is_can_fd_enabled = 0;


	int client_fd;
    struct sockaddr_can addr;
	struct ifreq ifr;
    if ((client_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        return -1;
    }
 
	strcpy(ifr.ifr_name, "vcan0");
	ioctl(client_fd, SIOCGIFINDEX, &ifr);

	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	
	if(bind(client_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return -1;

	config->controller->base_address = client_fd;
	return 0;
}

static int can_posix_driver_set_baudrate(struct tpl_can_controller_t *ctrl, CanControllerBaudrateConfig *baud_rate_config)
{
	return 0;
}

static Std_ReturnType can_posix_driver_transmit(struct tpl_can_controller_t *ctrl, const Can_PduType *pdu_info)
{
	struct can_frame frame;
	uint32 i, indx = 0;
	int framelen = pdu_info->length;

	while (framelen >= CAN_PAYLOAD_SIZE) {
		memset(&frame, 0, sizeof(frame));
		frame.can_id = CAN_FRAME_ID;
		frame.can_dlc = CAN_PAYLOAD_SIZE;
		if(indx==0) {
			frame.data[0] = CAN_PT_HEAD1;
			frame.data[1] = CAN_PT_HEAD2;
		}
		for (i = 0; i < frame.can_dlc; i++) {
			if(indx == 0 && (i == 0 || i == 1)) continue;
			frame.data[i] = pdu_info->sdu[i + indx*CAN_PAYLOAD_SIZE - 2];
		}			
		
		if(write(ctrl->base_address, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) return -1;
		framelen -= CAN_PAYLOAD_SIZE;
		indx++;
	}
	framelen+=2;
	if(framelen > 0) {
		memset(&frame, 0, sizeof(frame));
		frame.can_id = CAN_FRAME_ID;
		frame.can_dlc = framelen;
		for (i = 0; i < frame.can_dlc; i++)
			frame.data[i] = pdu_info->sdu[i + indx*CAN_PAYLOAD_SIZE -2];
		
		if(frame.can_dlc <= 6) {
			frame.can_dlc+=2;
			frame.data[framelen] = CAN_PT_TAIL1;
			frame.data[framelen + 1] = CAN_PT_TAIL2;
		}

		if(write(ctrl->base_address, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) return -1;

		if(framelen > 6) {
			memset(&frame, 0, sizeof(frame));
			frame.can_id = CAN_FRAME_ID;
			frame.can_dlc = 2;
			
			frame.data[framelen] = CAN_PT_TAIL1;
			frame.data[framelen + 1] = CAN_PT_TAIL2;

			if(write(ctrl->base_address, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) return -1;
		}
	}
	return 0;
}

static Std_ReturnType can_posix_driver_receive(struct tpl_can_controller_t *ctrl, Can_PduType *pdu_info)
{

	struct can_frame frame;
	int nbytes;
	uint32 i = 0;
	int indx = 0;
	nbytes = read(ctrl->base_address, &frame, sizeof(struct can_frame));
	pdu_info->id = frame.can_id;
	while(nbytes > 0) {
		if(indx == 0 && (frame.data[0] != CAN_PT_HEAD1 || frame.data[1] != CAN_PT_HEAD2)) {
			nbytes = read(ctrl->base_address, &frame, sizeof(struct can_frame));
			pdu_info->id = frame.can_id;
		}
		else {
			if((frame.data[frame.can_dlc - 2] == CAN_PT_TAIL1) && (frame.data[frame.can_dlc - 1] == CAN_PT_TAIL2)) {
				for(i = 0; i < (uint32) (frame.can_dlc - 2); i++) {
					if(indx == 0 && (i==0 || i==1)) continue;
					pdu_info->sdu[i + indx*CAN_PAYLOAD_SIZE - 2] = frame.data[i];
				}	
				pdu_info->length = indx*CAN_PAYLOAD_SIZE + frame.can_dlc - 4;

				return E_OK;
			}
			for(i = 0; i < frame.can_dlc; i++) {
				if(indx == 0 && (i==0 || i==1)) continue;
				pdu_info->sdu[i + indx*CAN_PAYLOAD_SIZE - 2] = frame.data[i];
			}
			indx++;
			do {
				nbytes = read(ctrl->base_address, &frame, sizeof(struct can_frame));
			} while(pdu_info->id != frame.can_id);
		}
	}
	
	return E_NOT_OK;	
}

static int can_posix_driver_is_data_available(struct tpl_can_controller_t *ctrl)
{
	int count = 0;
	ioctl(ctrl->base_address, FIONREAD, &count);

	if(count > 0) return 1;

	return 0;
}
