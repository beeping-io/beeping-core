OBJS = \
  ./src/BeepingCoreLib_api.o \
  ./src/Encoder.o \
  ./src/Decoder.o \
  ./src/EncoderAudible.o \
  ./src/DecoderAudible.o \
  ./src/EncoderNonAudible.o \
  ./src/DecoderNonAudible.o \
  ./src/EncoderAudibleMultiTone.o \
  ./src/DecoderAudibleMultiTone.o \
  ./src/EncoderNonAudibleMultiTone.o \
  ./src/DecoderNonAudibleMultiTone.o \
  ./src/EncoderHiddenMultiTone.o \
  ./src/DecoderHiddenMultiTone.o \
  ./src/EncoderCustomMultiTone.o \
  ./src/DecoderCustomMultiTone.o \
  ./src/DecoderAllMultiTone.o \
  ./src/Globals.o \
  ./src/SpectralAnalysis.o \
  ./src/ReedSolomon.o

all: libBeepingCore

DEPS=$(OBJS:.o=.d)

depend: $(DEPS)

include $(DEPS)

libBeepingCore: $(OBJS)  
	ar rcs $@.a $(OBJS)

clean:
	rm -f $(OBJS) $(DEPS) libBeepingCore.a

CXXFLAGS= -w -DLINUX -I./include -I. -I././src//fftooura \
          -O3 -DNDEBUG

%.o: %.c
	g++ $(CXXFLAGS) -c -o $@ $<

%.d: %.c
	echo $(@:.d=.o): \\> $@
	g++ $(CXXFLAGS) -M -o - $< | sed s/.*:// >> $@

%.o: %.cxx
	g++ $(CXXFLAGS) -c -o $@ $<

%.d: %.cxx
	echo $(@:.d=.o): \\> $@
	g++ $(CXXFLAGS) -M -o - $< | sed s/.*:// >> $@

%.o: %.cpp
	g++ $(CXXFLAGS) -c -o $@ $<

%.d: %.cpp
	echo $(@:.d=.o): \\> $@
	g++ $(CXXFLAGS) -M -o - $< | sed s/.*:// >> $@ 
