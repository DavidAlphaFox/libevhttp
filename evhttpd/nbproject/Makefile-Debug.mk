#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/HttpUtils.o \
	${OBJECTDIR}/http_1.1/Http11Handler.o \
	${OBJECTDIR}/HttpServer.o \
	${OBJECTDIR}/HttpHandlerManager.o \
	${OBJECTDIR}/RWIOPump.o \
	${OBJECTDIR}/ChunkedBodyBuilder.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/HttpHandler.o \
	${OBJECTDIR}/EvNIOWriter.o \
	${OBJECTDIR}/CleanerTimer.o \
	${OBJECTDIR}/event/EventDispatcher.o \
	${OBJECTDIR}/SocketUtils.o \
	${OBJECTDIR}/Dispatcher.o \
	${OBJECTDIR}/http_1.0/Http10HandlerFactory.o \
	${OBJECTDIR}/SocketNIOWriter.o \
	${OBJECTDIR}/http_1.0/Http10Handler.o \
	${OBJECTDIR}/MemoryPool.o \
	${OBJECTDIR}/SendfileIOPump.o \
	${OBJECTDIR}/RequestData.o \
	${OBJECTDIR}/StringUtils.o \
	${OBJECTDIR}/Request.o \
	${OBJECTDIR}/EvNIOReader.o \
	${OBJECTDIR}/SocketNIOReader.o \
	${OBJECTDIR}/AbstractIOPump.o \
	${OBJECTDIR}/HttpProcess.o \
	${OBJECTDIR}/HttpHandlerFactory.o \
	${OBJECTDIR}/Timer.o \
	${OBJECTDIR}/ProcessSignal.o \
	${OBJECTDIR}/HttpServletManager.o \
	${OBJECTDIR}/http_1.1/Http11HandlerFactory.o \
	${OBJECTDIR}/test/test.o \
	${OBJECTDIR}/Response.o \
	${OBJECTDIR}/Process.o \
	${OBJECTDIR}/ProcessFDSender.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/evhttpd

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/evhttpd: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/evhttpd ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/HttpUtils.o: HttpUtils.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/HttpUtils.o HttpUtils.cpp

${OBJECTDIR}/http_1.1/Http11Handler.o: http_1.1/Http11Handler.cpp 
	${MKDIR} -p ${OBJECTDIR}/http_1.1
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/http_1.1/Http11Handler.o http_1.1/Http11Handler.cpp

${OBJECTDIR}/HttpServer.o: HttpServer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/HttpServer.o HttpServer.cpp

${OBJECTDIR}/HttpHandlerManager.o: HttpHandlerManager.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/HttpHandlerManager.o HttpHandlerManager.cpp

${OBJECTDIR}/RWIOPump.o: RWIOPump.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/RWIOPump.o RWIOPump.cpp

${OBJECTDIR}/ChunkedBodyBuilder.o: ChunkedBodyBuilder.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/ChunkedBodyBuilder.o ChunkedBodyBuilder.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/HttpHandler.o: HttpHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/HttpHandler.o HttpHandler.cpp

${OBJECTDIR}/EvNIOWriter.o: EvNIOWriter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/EvNIOWriter.o EvNIOWriter.cpp

${OBJECTDIR}/CleanerTimer.o: CleanerTimer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/CleanerTimer.o CleanerTimer.cpp

${OBJECTDIR}/event/EventDispatcher.o: event/EventDispatcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/event
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/event/EventDispatcher.o event/EventDispatcher.cpp

${OBJECTDIR}/SocketUtils.o: SocketUtils.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/SocketUtils.o SocketUtils.cpp

${OBJECTDIR}/Dispatcher.o: Dispatcher.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/Dispatcher.o Dispatcher.cpp

${OBJECTDIR}/http_1.0/Http10HandlerFactory.o: http_1.0/Http10HandlerFactory.cpp 
	${MKDIR} -p ${OBJECTDIR}/http_1.0
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/http_1.0/Http10HandlerFactory.o http_1.0/Http10HandlerFactory.cpp

${OBJECTDIR}/SocketNIOWriter.o: SocketNIOWriter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/SocketNIOWriter.o SocketNIOWriter.cpp

${OBJECTDIR}/http_1.0/Http10Handler.o: http_1.0/Http10Handler.cpp 
	${MKDIR} -p ${OBJECTDIR}/http_1.0
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/http_1.0/Http10Handler.o http_1.0/Http10Handler.cpp

${OBJECTDIR}/MemoryPool.o: MemoryPool.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/MemoryPool.o MemoryPool.cpp

${OBJECTDIR}/SendfileIOPump.o: SendfileIOPump.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/SendfileIOPump.o SendfileIOPump.cpp

${OBJECTDIR}/RequestData.o: RequestData.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/RequestData.o RequestData.cpp

${OBJECTDIR}/StringUtils.o: StringUtils.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/StringUtils.o StringUtils.cpp

${OBJECTDIR}/Request.o: Request.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/Request.o Request.cpp

${OBJECTDIR}/EvNIOReader.o: EvNIOReader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/EvNIOReader.o EvNIOReader.cpp

${OBJECTDIR}/SocketNIOReader.o: SocketNIOReader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/SocketNIOReader.o SocketNIOReader.cpp

${OBJECTDIR}/AbstractIOPump.o: AbstractIOPump.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/AbstractIOPump.o AbstractIOPump.cpp

${OBJECTDIR}/HttpProcess.o: HttpProcess.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/HttpProcess.o HttpProcess.cpp

${OBJECTDIR}/HttpHandlerFactory.o: HttpHandlerFactory.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/HttpHandlerFactory.o HttpHandlerFactory.cpp

${OBJECTDIR}/Timer.o: Timer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/Timer.o Timer.cpp

${OBJECTDIR}/ProcessSignal.o: ProcessSignal.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/ProcessSignal.o ProcessSignal.cpp

${OBJECTDIR}/HttpServletManager.o: HttpServletManager.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/HttpServletManager.o HttpServletManager.cpp

${OBJECTDIR}/http_1.1/Http11HandlerFactory.o: http_1.1/Http11HandlerFactory.cpp 
	${MKDIR} -p ${OBJECTDIR}/http_1.1
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/http_1.1/Http11HandlerFactory.o http_1.1/Http11HandlerFactory.cpp

${OBJECTDIR}/test/test.o: test/test.cpp 
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/test/test.o test/test.cpp

${OBJECTDIR}/Response.o: Response.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/Response.o Response.cpp

${OBJECTDIR}/Process.o: Process.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/Process.o Process.cpp

${OBJECTDIR}/ProcessFDSender.o: ProcessFDSender.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/ProcessFDSender.o ProcessFDSender.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/evhttpd

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
