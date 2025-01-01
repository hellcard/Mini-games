void fatal(char* mes){
	char error_mes[100];

	strcpy(error_mes,"[ERROR] ");
	strcpy(error_mes,mes);
	perror(error_mes);
	exit(-1);
}

void *ptr_malloc(unsigned int size){
	void *ptr;
	ptr = malloc(size);
	if(ptr == NULL) fatal("In ptr_malloc function - func.h");
	return ptr;
}
