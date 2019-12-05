namespace xmonk {

class mydspSIG0 {
	
  private:
	
	int iRec0[2];
	
  public:
	
	int getNumInputsmydspSIG0() {
		return 0;
		
	}
	int getNumOutputsmydspSIG0() {
		return 1;
		
	}
	int getInputRatemydspSIG0(int channel) {
		int rate;
		switch (channel) {
			default: {
				rate = -1;
				break;
			}
			
		}
		return rate;
		
	}
	int getOutputRatemydspSIG0(int channel) {
		int rate;
		switch (channel) {
			case 0: {
				rate = 0;
				break;
			}
			default: {
				rate = -1;
				break;
			}
			
		}
		return rate;
		
	}
	
	void instanceInitmydspSIG0(int samplingFreq) {
		for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) {
			iRec0[l0] = 0;
			
		}
		
	}
	
	void fillmydspSIG0(int count, double* output) {
		for (int i = 0; (i < count); i = (i + 1)) {
			iRec0[0] = (iRec0[1] + 1);
			output[i] = std::sin((9.5873799242852573e-05 * double((iRec0[0] + -1))));
			iRec0[1] = iRec0[0];
			
		}
		
	}

};


class Dsp {
private:
	uint32_t fSamplingFreq;
	FAUSTFLOAT fHslider0;
	FAUSTFLOAT	*fHslider0_;
	double fConst0;
	double fConst1;
	double fRec1[2];
	double fConst2;
	double fConst3;
	FAUSTFLOAT fHslider1;
	FAUSTFLOAT fCheckbox0;
	FAUSTFLOAT *fCheckbox1_;
	FAUSTFLOAT fCheckbox1;
	FAUSTFLOAT fCheckbox2;
	FAUSTFLOAT fCheckbox3;
	double fRec3[2];
	double fConst4;
	double fRec4[2];
	double fConst5;
	FAUSTFLOAT fHslider2;
	FAUSTFLOAT	*fHslider2_;
	double fRec6[2];
	double fRec2[3];
	double fRec7[3];
	double fRec8[3];
	double fRec9[3];
	double fRec10[3];
	double fRec11[3];
	double TET;
	double ref_freq;
	double ref_note;
	double max_note;
	mydspSIG0* sig0;
	double ftbl0mydspSIG0[65536];

	void connect(uint32_t port,void* data);
	void clear_state_f();
	void init(uint32_t samplingFreq);
	void compute(int count, FAUSTFLOAT *output0, FAUSTFLOAT *output1);
public:
	double gate;
	double note;
	double vowel;
	double panic;
	double velocity;
	double sustain;
	double gain;
	static void clear_state_f_static(Dsp*);
	static void init_static(uint32_t samplingFreq, Dsp*);
	static void compute_static(int count, FAUSTFLOAT *output0, FAUSTFLOAT *output1, Dsp*);
	static void del_instance(Dsp *p);
	static void connect_static(uint32_t port,void* data, Dsp *p);
	Dsp();
	~Dsp();
};

} // end namespace xmonk
