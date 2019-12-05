namespace stereodelay {

class Dsp {
private:
	uint32_t fSamplingFreq;
	int IOTA;
	double fVec0[65536];
	double fConst0;
	double fRec0[2];
	double fRec1[2];
	double fRec2[2];
	double fRec3[2];
	double fVec1[65536];

	void connect(uint32_t port,void* data);
	void clear_state_f();
	void init(uint32_t samplingFreq);
	void compute(int count, FAUSTFLOAT *input0, FAUSTFLOAT *input1, FAUSTFLOAT *output0, FAUSTFLOAT *output1);
public:
	static void clear_state_f_static(Dsp*);
	static void init_static(uint32_t samplingFreq, Dsp*);
	static void compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *input1, FAUSTFLOAT *output0, FAUSTFLOAT *output1, Dsp*);
	static void del_instance(Dsp *p);
	static void connect_static(uint32_t port,void* data, Dsp *p);
	Dsp() {};
	~Dsp() {};
};

} // end namespace stereodelay
