/* needs to stay the first line */
asm(".code16gcc\n");
asm("call main;");

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

#define INT_DIGIT_COUNT 11

void printd(int x) {
  if(x == 0) {
    putc('0');
    return;
  }
  char s[INT_DIGIT_COUNT];
  s[INT_DIGIT_COUNT - 1] = 0;
  int i;
  for(i = INT_DIGIT_COUNT - 2; x!=0; x/=10)
    s[i--] = (x % 10) + '0';
  puts(s + i + 1);
}

int printf(char* format, ...) {
  int* arg = (int*) &format;
  arg++;
  char c;

  int x;
  while((c = *format++)) {
    if (c == '%') {
      switch(c = *format++) {
      case 'c':
	putc((char) *arg);
	break;
      case 's':
	puts((char*) *arg);
	break;
      case 'd':
	x = *arg;
	if (x < 0) {
	  putc('-');
	  x = -x;
	}
	printd(x);
	break;
      case 'u':
	printd(*arg);
	break;
      case '%':
	putc('%');
	break;
      default:
	puts("Invalid format string\n");
	return 1;
      }
      arg++;
    }
    else {
      putc(c);
    }
  }

  return 0;
}

char getc() {
  char c;
  asm("mov $0x0, %%eax;"
      "int $0x16;"
      "mov %%al, %0;"
      : "=r"(c)
      :: "eax");
  return c;
}


void getpw(char* buf, int bufsize) {
  int i;
  char c = 0;

  if(bufsize <= 1)
    return;
  
  for(i=0; i<bufsize-1; i++) {
    c = getc();
    if(c == '\r') {
      buf[i] = 0;
      break;
    }
    putc('.');
    buf[i] = c;
  }
  buf[bufsize - 1] = 0;

  while(c != '\r')
    c = getc();
  puts("\n\r");
}

#define PW_SIZE 8
void main(void)
{
  char s[PW_SIZE + 1];
  
  printf("Hello!\n\r");
  do {
    getpw(s, PW_SIZE + 1);
    if(s[0] != 0) {
      puts(s);
      puts("\n\r");
    }
  } while(s[0] != 0);
  
  char* message = "Now a little printf demo!";
  printf("%s\n\r", message);
  printf("%c = %c%c^%d\n\r", 'e', 'm', 'c', 2);
  
  printf("Reboot!\n\r\n\n");
  asm("int $0x19");

  // If for some reason our system does not restart, the function will return to the bootloader
}
