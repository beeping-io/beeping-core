OBJS = \
  ../BeepingCoreLib/src/BeepingCoreLib_api.o \
  ../BeepingCoreLib/src/Encoder.o \
  ../BeepingCoreLib/src/Decoder.o \
  ../BeepingCoreLib/src/EncoderAudible.o \
  ../BeepingCoreLib/src/DecoderAudible.o \
  ../BeepingCoreLib/src/EncoderNonAudible.o \
  ../BeepingCoreLib/src/DecoderNonAudible.o \
  ../BeepingCoreLib/src/EncoderAudibleMultiTone.o \
  ../BeepingCoreLib/src/DecoderAudibleMultiTone.o \
  ../BeepingCoreLib/src/EncoderNonAudibleMultiTone.o \
  ../BeepingCoreLib/src/DecoderNonAudibleMultiTone.o \
  ../BeepingCoreLib/src/EncoderHiddenMultiTone.o \
  ../BeepingCoreLib/src/DecoderHiddenMultiTone.o \
  ../BeepingCoreLib/src/EncoderCustomMultiTone.o \
  ../BeepingCoreLib/src/DecoderCustomMultiTone.o \
  ../BeepingCoreLib/src/DecoderAllMultiTone.o \
  ../BeepingCoreLib/src/Globals.o \
  ../BeepingCoreLib/src/SpectralAnalysis.o \
  ../BeepingCoreLib/src/ReedSolomon.o

all: libBeepingCoreUniversal

DEPS=$(OBJS:.o=.d)

depend: $(DEPS)

include $(DEPS)

libBeepingCoreUniversal: $(OBJS)  
	ar rcs $@.a $(OBJS)

clean:
	rm -f $(OBJS) $(DEPS) libBeepingCoreUniversal.a

CXXFLAGS= -w -DLINUX -I../BeepingCoreLib/include -I. -I../BeepingCoreLib/src/fftooura \
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
