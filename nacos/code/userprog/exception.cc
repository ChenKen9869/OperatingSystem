// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void CreateFunc(){
    int base = machine->ReadRegister(4);
    int value;
    int count = 0;
    //determine the filename length first
    do {
        machine -> ReadMem(base++,1,&value);
        count ++;
    }while(value != 0);
    printf ("filenname length is %d\n",count);
    //now getting the filename !
    base = base - count;
    char fileName[count];
    for (int i = 0;i < count;i ++){
        machine -> ReadMem(base+i,1,&value);
        fileName[i] = (char)value;
    }
    fileSystem->Create(fileName,1024);
    //PC前移：
    machine->PCAdvanced();
}

void OpenFunc(){
    int base = machine->ReadRegister(4);
    int value;
    int count = 0;
    //determine the filename length first
    do {
        machine -> ReadMem(base++,1,&value);
        count ++;
    }while(value != 0);
    printf ("filenname length is %d\n",count);
    //now getting the filename !
    base = base - count;
    char fileName[count];
    for (int i = 0;i < count;i ++){
        machine -> ReadMem(base+i,1,&value);
        fileName[i] = (char)value;
    }
    OpenFile *openfile = fileSystem -> Open(fileName);
    if(openfile == NULL) {
        printf("file is not exist!open failed ..\n");
    }
    else{
        printf("open succeed!\n");
        //put the return value into 2-register
        machine -> WriteRegister(2,(int)openfile);
        printf("openfile pointer = %d\n",(int)openfile);
    }
    //PC前移：
    machine->PCAdvanced();
}

void CloseFunc(){
    int base = machine->ReadRegister(4);
    printf("base=openfile pointer=%d\n",base);
    OpenFile *openfile = (OpenFile *)base;
    printf("close the userprog-file now]\n");
    //PC前移：
    machine->PCAdvanced();
}

int ReadFunc(){
    //getting write content start pointer
    int buffer_base = machine->ReadRegister(4);
    //getting write content length
    int count = machine->ReadRegister(5);
    //getting the openfile pointer
    int fd = machine->ReadRegister(6);

    printf("reading content length is %d\n",count);
    printf("reading the userprog-file now\n");
    OpenFile *openfile = (OpenFile *)fd;
    printf("reading openfile=%d\n",fd);
    printf("reading base=%d\n",buffer_base);
    if(openfile == NULL) {
        printf("file is not exist!open failed..\n");
    }
    else{
        printf("open succeed!reading..\n");
        char content[count];
        int value;
        openfile->Read(content,count);
        content[count] = '\0';
        printf("reading content: %s\n",content);
        for(int i = 0;i < count;i++){
            value = (int)content[i];
            machine->WriteMem(buffer_base+i,1,value);
        }
    }
}

void WriteFunc(){
    //getting write content start pointer
    int buffer_base = machine->ReadRegister(4);
    //getting write content length
    int count = machine->ReadRegister(5);
    //getting the openfile pointer
    int fd = machine->ReadRegister(6);
    int value;
    int count1 = 0;
    //determine the filename length first
    do {
        machine -> ReadMem(buffer_base++,1,&value);
        count1 ++;
    }while(value != 0);
    printf ("filenname length is %d\n",count1);
    //now getting the filename !
    buffer_base = buffer_base - count1;
    char fileName[count1];
    for (int i = 0;i < count1;i ++){
        machine -> ReadMem(buffer_base+i,1,&value);
        fileName[i] = (char)value;
    }
    printf("writing content length is %d\n",count);
    //getting write content
    char content[count];
    for(int i = 0;i < count;i ++){
        machine->ReadMem(buffer_base+i,1,&value);
        content[i] = (char)value;
    }
    printf("write content: %s\n",content);
    printf("write the userprog-file %s now\n",fileName);
    OpenFile *openfile = (OpenFile *)fd;
    if(openfile == NULL) {
        printf("file is not exist!open failed..\n");
    }
    else {
        printf("open succeed! writing..\n");
        //write in the file
        openfile -> Write(content,count);
    }
}


//exercise4,5
//exec
void exec_fork_func(int name)
{
    char *fileName = new char[256];
    fileName = (char *)name;
    OpenFile *ex2 = fileSystem->Open(fileName);
    AddrSpace *space2 = new AddrSpace(ex2);
    currentThread->space = space2;
    currentThread->filename = fileName;
    space2->InitRegisters();
    space2->RestoreState();
    printf("thread %s starts running\n",currentThread->getName());
    machine->Run();
}

void
ExecFunc(){
    //name pointer
    int base = machine->ReadRegister(4);
    int value;
    int count = 0;
    //determine the filename length first
    do{
        machine->ReadMem(base++,1,&value);
        count++;
    }while(value != 0);
    printf("filename length is %d\n",count);
    //now getting the filename
    base = base - count;
    char fileName[count];
    for(int i = 0;i < count; i ++){
        machine->ReadMem(base+i,1,&value);
        fileName[i] = (char)value;
    }
    fileName[count] = '\0';
    
    Thread *newthread = new Thread("second thread",1);
    //set relationship pointer
    bool found = false;
    for (int i = 0; i < MaxThreadSize;i++){
        if (currentThread->childThread[i] == NULL){
            currentThread->childThread[i] = newthread;
            machine -> WriteRegister(2,(int)newthread);
            found = true;
            break;
        }
    }
    if(!found){
        printf("current thread is full of children! Exec failed!\n");
        machine->PCAdvanced();
        return;
    }
    newthread->fatherThread = currentThread;
    newthread->Fork(exec_fork_func,(int)fileName);
    machine->PCAdvanced;
}

//join
void 
JoinFunc(){
    int id = machine->ReadRegister(4);
    Thread *cthread = (Thread *)id;
    bool found = false;
    int num;
    for(int i = 0;i < MaxThreadSize;i ++){
        if(currentThread->childThread[i] == cthread){
            num = i;
            found = true;
            break;
        }
    }
    if(!found){
        printf("cannot find this thread! Join failed!\n");
        return;
    }
    while(currentThread->childThread[num] != NULL){
        currentThread->Yield();
    }
    printf("childthread is finished! Join successed")
    machine->PCAdvanced();
}

//exit
void
ExitFunc(){
    if (currentThread->getName() != "main"){
        if(currentThread->fatherThread != NULL){
            currentThread->fatherThread->childThread = NULL;
        }
        currentThread->Finish();
    }
    else{
        machine->PCAdvanced();
    }
}
//fork
void
fork_func(int s)
{
    printf("%s is now preparing for fork..\n",currentThread->getName());
    //set the new thread user space, the same as father thread
    space_pc *spacewithpc = (space_pc *)s;
    AddrSpace *tmp = spacewithpc->space;
    AddrSpace *space = new AddrSpace(tmp);
    currentThread->space = space;

    int PC_now = spacewithpc->PC_now;
    //set the PC register to the func it will execute
    machine -> WriteRegister(PCReg,PC_now);
    machine -> WriteRegister(NextPCReg,PC_now+4);
    
    //set the new thread registers, the same as father thread
    currentThread->SaveUserState();
    printf("Everything is OK! thread %s starts running..\n",currentThread->getName());
    machine -> Run();
}

void
ForkFunc()
{
    //get the start position of the function
    int PC_now = machine->ReadRegister(4);
    space_pc *spacewithpc = new space_pc;
    spacewithpc->space = currentThread->space;
    spacewithpc->PC_now = PC_now;

    Thread *newthread = new Thread("second thread",1);
    //set relationship pointer
    bool found = false;
    for(int i = 0;i < MaxThreadSize; i ++){
        if (currentThread->childThread[i] = NULL){
            currentThread->childThread[i] = newthread;
            machine->WriteRegister(2,(int)newthread);
            found = true;
            break;
        }
    }
    if(!found){
        printf("current thread is full of children！\n");
        machine -> PCAdvanced();
        return;
    }
    newthread->fatherThread = currentThread;
    newthread->Fork(fork_func,(int)spacewithpc);
    machine->PCAdvanced();
}
//yield
void 
YieldFunc(){
    machine->PCAdvanced();
    printf("%s is now yield!\n",currentThread->getName());
    currentThread->Yield();
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
    } 
    else if ((which == SyscallException) && (type == SC_Create)){
    DEBUG('a', "Create systemcall.\n");
   	CreateFunc();
    }
    else if ((which == SyscallException) && (type == SC_Open)){
    DEBUG('a', "Open systemcall.\n");
   	OpenFunc();
    }
    else if ((which == SyscallException) && (type == SC_Read)){
    DEBUG('a', "Read systemcall.\n");
   	ReadFunc();
    }
    else if ((which == SyscallException) && (type == SC_Write)){
    DEBUG('a', "Write systemcall.\n");
   	WriteFunc();
    }
    else if ((which == SyscallException) && (type == SC_Close)){
    DEBUG('a', "Close systemcall.\n");
   	CloseFunc();
    }

    else if ((which == SyscallException) && (type == SC_Exec)){
    DEBUG('a', "Exec systemcall.\n");
   	ExecFunc();
    }
    else if ((which == SyscallException) && (type == SC_Fork)){
    DEBUG('a', "Fork systemcall.\n");
   	ForkFunc();
    }
    else if ((which == SyscallException) && (type == SC_Yield)){
    DEBUG('a', "Yield systemcall.\n");
   	YieldFunc();
    }
    else if ((which == SyscallException) && (type == SC_Join)){
    DEBUG('a', "Join systemcall.\n");
   	JoinFunc();
    }
    else if ((which == SyscallException) && (type == SC_Exit)){
    DEBUG('a', "Exit systemcall.\n");
   	ExitFunc();
    }


    else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}
