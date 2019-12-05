namespace stereoverb {

class Dsp {
private:
	uint32_t fSamplingFreq;
	double fRec9[2];
	int IOTA;
	double fVec0[2048];
	double fRec8[2];
	double fRec11[2];
	double fVec1[2048];
	double fRec10[2];
	double fRec13[2];
	double fVec2[2048];
	double fRec12[2];
	double fRec15[2];
	double fVec3[2048];
	double fRec14[2];
	double fRec17[2];
	double fVec4[2048];
	double fRec16[2];
	double fRec19[2];
	double fVec5[2048];
	double fRec18[2];
	double fRec21[2];
	double fVec6[2048];
	double fRec20[2];
	double fRec23[2];
	double fVec7[2048];
	double fRec22[2];
	double fVec8[1024];
	double fRec6[2];
	double fVec9[512];
	double fRec4[2];
	double fVec10[512];
	double fRec2[2];
	double fVec11[256];
	double fRec0[2];
	double fRec33[2];
	double fVec12[2048];
	double fRec32[2];
	double fRec35[2];
	double fVec13[2048];
	double fRec34[2];
	double fRec37[2];
	double fVec14[2048];
	double fRec36[2];
	double fRec39[2];
	double fVec15[2048];
	double fRec38[2];
	double fRec41[2];
	double fVec16[2048];
	double fRec40[2];
	double fRec43[2];
	double fVec17[2048];
	double fRec42[2];
	double fRec45[2];
	double fVec18[2048];
	double fRec44[2];
	double fRec47[2];
	double fVec19[2048];
	double fRec46[2];
	double fVec20[1024];
	double fRec30[2];
	double fVec21[512];
	double fRec28[2];
	double fVec22[512];
	double fRec26[2];
	double fVec23[256];
	double fRec24[2];

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

} // end namespace stereoverb
