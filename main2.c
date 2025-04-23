#include <stdint.h>
#include <stdio.h>
#include <linux/usb/ch9.h>
#include <linux/usbdevice_fs.h>
#include <fcntl.h>
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
	get_configuration = 8,
	set_descriptor = 7
} DeviceRequest;

typedef enum : uint8_t {
	string = 3
} DescriptorType ;

int main() {
	int my_fd = open("/dev/bus/usb/001/030", O_RDWR);

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

	uint16_t value = (string << 8) | 0;
	uint8_t lang_buf[1024];
	struct usbdevfs_ctrltransfer lang_request = {
		.bRequestType = req_type.bits,
		.bRequest = get_descriptor,
		.wValue = value,
		.wIndex = 0,
		.wLength = sizeof(lang_buf),
		.timeout = 1000,
		.data = &lang_buf
	};
	int n = ioctl(my_fd, USBDEVFS_CONTROL, &lang_request);
	printf("ret n: %d\n", n);
	if (n < 0) perror("GET LANGIDs");  // parse lang_buf as two-byte LANGIDs
	int size_lang = lang_buf[0];
	for (int i = 2; i < (size_lang-2)/2; i++) {
		printf("%c", lang_buf[i] | lang_buf[i+1] << 8);
	}
	

	uint16_t langid = lang_buf[2] | (lang_buf[3]<<8); // get first language
	uint8_t string_buf[1024];
	printf("lanf: %x\n", langid);

	value = (string << 8) | usb_desc.manufacturer;
	struct usbdevfs_ctrltransfer str_manu_request = {
		.bRequestType = req_type.bits,
		.bRequest = get_descriptor,
		.wValue = value,
		.wIndex = langid,
		.wLength = sizeof(string_buf),
		.timeout = 1000,
		.data = &string_buf
	};
	int ret = ioctl(my_fd, USBDEVFS_CONTROL, &str_manu_request);
	if (ret < 0) perror("GET STRING");
		int size = string_buf[0];
	for (int i = 2; i < size; i++) {
		printf("%c", string_buf[i]);
	}
	printf(" ");

	value = (string << 8) | usb_desc.product;
	struct usbdevfs_ctrltransfer str_prod_req = {
		.bRequestType = req_type.bits,
		.bRequest = get_descriptor,
		.wValue = value,
		.wIndex = langid,
		.wLength = sizeof(string_buf),
		.timeout = 1000,
		.data = &string_buf
	};
	ret = ioctl(my_fd, USBDEVFS_CONTROL, &str_prod_req);
	if (ret < 0) perror("GET STRING");
	size = string_buf[0];
	for (int i = 2; i < size; i++) {
		printf("%c", string_buf[i]);
	}
	printf(" - ");

	value = (string << 8) | usb_desc.serial_number;
	struct usbdevfs_ctrltransfer str_serial_req = {
		.bRequestType = req_type.bits,
		.bRequest = get_descriptor,
		.wValue = value,
		.wIndex = langid,
		.wLength = sizeof(string_buf),
		.timeout = 1000,
		.data = &string_buf
	};
	ret = ioctl(my_fd, USBDEVFS_CONTROL, &str_serial_req);
	if (ret < 0) perror("GET STRING");
	size = string_buf[0];
	for (int i = 2; i < size; i++) {
		printf("%c", string_buf[i]);
	}
	printf("\n");

	close(my_fd);
	return 0;
}
