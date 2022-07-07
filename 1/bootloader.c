/* needs to stay the first line */
asm(".code16gcc\njmp $0, $main");

extern int load_from_disk();

void putc(char c) {
  asm("mov $0xE, %%ah;"
      "mov %0, %%al;"
      "int $0x10;"
      :: "r"(c)
      : "eax"
      );
}

// s must point to a null-terminated ascii string
void puts(char* s) {
  for(int i=0; s[i]!=0; i++)
    putc(s[i]);
}

void main(void)
{
  
  /* We have to put the stack to a place, where it doesn't collide
     with the code we load.
   */
  // Set stack pointer
  
  asm("mov $0x7c00, %bp;"
      "mov %bp, %sp;");

  // Set segments
  asm("xor %ax, %ax;"
      "mov %ax, %ds;"
      "mov %ax, %es;"
      "mov %ax, %ss;");
		  
  puts("Loading from hard drive\n\r");

  int r = load_from_disk(2);
  
  if(r == 1) {
    puts("Disk Error\n\r");
    asm("jmp .");
  } else if (r == 2) {
    puts("Sector Error\n\r");
    asm("jmp .");
  }
  
  // Function for printing Hex
  // Note that the order differs from hexdump, maybe because of LE
  /*
  for(char* c = (char*) 0x7E00; c < (char*) 0x7EA0; c++) {
    
    if((int) c % 16 == 0)
      puts("\n\r");

    char n = (*c & 0xF0) / 16;
    n += 0x30;
    if(n > 0x39) n += 7;
    putc(n);
    
    n = *c & 0x0F;
    n += 0x30;
    if(n > 0x39) n += 7;
    putc(n);
    putc(' ');
  }*/

  // Calling code from hard drive
  puts("Calling code from hard drive\n\r");
  asm("mov $0x7E00, %eax;"
      "call *%eax;");

  // This will usually be called in error or debug cases
  puts("Kernel returned");
  asm("jmp .");

}
