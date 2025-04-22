#include <stdint.h>
#include <stdio.h>
#include <linux/usb/ch9.h>
#include <linux/usbdevice_fs.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>

#define MAX_SIZE 10000
#define data_size 18

typedef struct {
	uint8_t lenght;
	uint8_t descriptor_type;
	uint16_t bcd_usb;
	uint8_t class;
	uint8_t sub_class;
	uint8_t protocol;
	uint8_t max_packet_size;
	uint16_t vendor_id;
	uint16_t product_id;
	uint16_t bcd_device;
	uint8_t manufacturer;
	uint8_t product;
	uint8_t serial_number;
	uint8_t num_configurations;
} UsbDescriptor;

void printUsbDescriptor(UsbDescriptor usbDesc) {
	printf("lenght: %x\n", usbDesc.lenght);
	printf("descriptor_type: %x\n", usbDesc.descriptor_type);
	printf("bcd_usb: %x\n", usbDesc.bcd_usb);
	printf("class: %x\n", usbDesc.class);
	printf("sub_class: %x\n", usbDesc.sub_class);
	printf("protocol: %x\n", usbDesc.protocol);
	printf("max_packet_size: %x\n", usbDesc.max_packet_size);
	printf("vendor_id: %x\n", usbDesc.vendor_id);
	printf("product_id: %x\n", usbDesc.product_id);
	printf("bcd_device: %x\n", usbDesc.bcd_device);
	printf("manufacturer: %x\n", usbDesc.manufacturer);
	printf("product: %x\n", usbDesc.product);
	printf("serial_number: %x\n", usbDesc.serial_number);
	printf("num_configurations: %x\n", usbDesc.num_configurations);
}

typedef enum : uint8_t {
	host_to_device = 0,
	device_to_host = 1
} Direction;

typedef enum : uint8_t {
	standard = 0,
	class = 1,
	vendor = 2,
	reserved = 3,
} Type;

typedef enum: uint8_t {
	device = 0,
	interface = 1,
	endpoint = 2,
	other = 3,
} Recipient;

typedef union {
	struct {
		Recipient recipient : 5;
		Type type : 2;
		Direction direction : 1;
	} field;
	uint8_t bits;
} RequestType;

typedef enum : uint8_t {
	 get_descriptor = 6,
	 get_configuration = 8
} DeviceRequest;

typedef enum : uint8_t {
	string = 3
} DescriptorType ;

int main() {
	int my_fd = open("/dev/bus/usb/003/002", O_RDWR);

	if (my_fd < 0) {
		perror("Failed to open device");
		return 1;
	}

	UsbDescriptor usb_desc;

	read(my_fd, &usb_desc, 18);

	printUsbDescriptor(usb_desc);

	RequestType req_type = {.field = {
		.recipient = device,
		.type = standard,
		.direction = device_to_host
	}};

	uint16_t value = (string << 8) | 1;
	printf("value: %b\n", value);


	char string_buf[128];
	printf("req bits: %b\n", req_type.bits);

	struct usbdevfs_ctrltransfer ctrl_request = {
		.bRequestType = req_type.bits,
		.bRequest = get_descriptor,
		.wValue = value,
		.wIndex = 0,
		.wLength = sizeof(string_buf),
		.timeout = 1000,
		.data = &string_buf
	};

	// struct usbdevfs_ctrltransfer config_request = {
	// 	.bRequestType = req_type.bits,
	// 	.bRequest = (int)get_configuration,
	// 	.wValue = 0,
	// 	.wIndex = 0,
	// 	.wLength = sizeof(string_buf),
	// 	.data = &string_buf
	// };

	int ret = ioctl(my_fd, USBDEVFS_CONTROL, &ctrl_request);
	printf("ret: %d\n", ret);
	if (ret < 0) {
		perror("USB control transfer failed");
	}

	int size = string_buf[0]-2;

	uint8_t* myText = malloc(sizeof(uint8_t)*size);

	for (int i = 0; i < size; i++) {
		myText[i] = string_buf[i+2];
		printf("%d, ", myText[i]);
	}
	printf("\n");
	for (int i = 2; i < size; i++) {
		printf("%c", string_buf[i]);
	}
	printf("\n");

	printf("string buf[0]: %d\n", string_buf[2] << 8 | string_buf[3]);
	printf("string buf: %s\n", string_buf);
	printf("string buf: %d\n", myText[1]);

	close(my_fd);
	return 0;
}
