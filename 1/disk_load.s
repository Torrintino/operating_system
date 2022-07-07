.code16
	
.section .text
.globl load_from_disk
.type load_from_disk, @function
load_from_disk:
	push %ebp
	mov %esp, %ebp
	
	push %bx
	push %cx
	push %dx

	mov $0x0, %ah
	int $0x13	

	// Number of sectors
	mov 8(%ebp), %al

	// 0x2 - Read sectors
	mov $0x2, %ah

	// Section number (starting from 1)
	mov $0x2, %cl

	// Boot device and head (0)
	// I want to mention, that I got help from stackoverflow,
	// regarding the value of %dl
	// https://stackoverflow.com/questions/55972474/controller-error-20h-for-int-13h-for-writing-a-bootloader
	// For some reason the BIOS doesn't seem to correctly set dl at boot
	mov $0x80, %dl
	mov $0, %dh

	// Cylinder
	mov $0x0, %ch

	// Dest Address
	mov $0x7E00, %bx
	
	int $0x13
	jc .disk_error
	cmp %al, 8(%ebp)
	jne .sector_error

	// Return the error type (0) Success, (1) Disk error, (2) Sector error
	mov 0x0, %ax
	jmp .exit

	.disk_error:
	mov $0x1, %ax
	jmp .exit

	.sector_error:
	mov $0x2, %ax

.exit:
	pop %dx
	pop %cx
	pop %bx
	pop %ebp
	ret

	
