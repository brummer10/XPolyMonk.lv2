// generated from file './/compressor.dsp' by dsp2cc:
// Code generated with Faust 2.15.11 (https://faust.grame.fr)


namespace compressor {

class Dsp {
private:
	uint32_t fSamplingFreq;
	double fConst0;
	double fConst1;
	double fConst2;
	double fConst3;
	double fRec1[2];
	double fConst4;
	double fRec0[2];
	double fRec3[2];
	double fRec2[2];

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


inline void Dsp::clear_state_f()
{
	for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) fRec1[l0] = 0.0;
	for (int l1 = 0; (l1 < 2); l1 = (l1 + 1)) fRec0[l1] = 0.0;
	for (int l2 = 0; (l2 < 2); l2 = (l2 + 1)) fRec3[l2] = 0.0;
	for (int l3 = 0; (l3 < 2); l3 = (l3 + 1)) fRec2[l3] = 0.0;
}

void Dsp::clear_state_f_static(Dsp *p)
{
	p->clear_state_f();
}

inline void Dsp::init(uint32_t samplingFreq)
{
	fSamplingFreq = samplingFreq;
	fConst0 = std::min<double>(192000.0, std::max<double>(1.0, double(fSamplingFreq)));
	fConst1 = std::exp((0.0 - (500.0 / fConst0)));
	fConst2 = std::exp((0.0 - (10.0 / fConst0)));
	fConst3 = (1.0 - fConst2);
	fConst4 = std::exp((0.0 - (2.0 / fConst0)));
	clear_state_f();
}

void Dsp::init_static(uint32_t samplingFreq, Dsp *p)
{
	p->init(samplingFreq);
}

void always_inline Dsp::compute(int count, FAUSTFLOAT *input0, FAUSTFLOAT *input1, FAUSTFLOAT *output0, FAUSTFLOAT *output1)
{
	for (int i = 0; (i < count); i = (i + 1)) {
		double fTemp0 = double(input0[i]);
		fRec1[0] = ((fConst2 * fRec1[1]) + (fConst3 * std::fabs(fTemp0)));
		double fTemp1 = ((fConst1 * double((fRec0[1] < fRec1[0]))) + (fConst4 * double((fRec0[1] >= fRec1[0]))));
		fRec0[0] = ((fRec0[1] * fTemp1) + (fRec1[0] * (1.0 - fTemp1)));
		double fTemp2 = std::max<double>(0.0, ((20.0 * (std::log10(fRec0[0]) + 1.0)) + 2.0));
		double fTemp3 = std::min<double>(1.0, std::max<double>(0.0, (0.49975012493753124 * fTemp2)));
		output0[i] = FAUSTFLOAT((std::pow(10.0, (0.050000000000000003 * ((fTemp2 * (0.0 - fTemp3)) / (fTemp3 + 1.0)))) * fTemp0));
		double fTemp4 = double(input1[i]);
		fRec3[0] = ((fConst2 * fRec3[1]) + (fConst3 * std::fabs(fTemp4)));
		double fTemp5 = ((fConst1 * double((fRec2[1] < fRec3[0]))) + (fConst4 * double((fRec2[1] >= fRec3[0]))));
		fRec2[0] = ((fRec2[1] * fTemp5) + (fRec3[0] * (1.0 - fTemp5)));
		double fTemp6 = std::max<double>(0.0, ((20.0 * (std::log10(fRec2[0]) + 1.0)) + 2.0));
		double fTemp7 = std::min<double>(1.0, std::max<double>(0.0, (0.49975012493753124 * fTemp6)));
		output1[i] = FAUSTFLOAT((std::pow(10.0, (0.050000000000000003 * ((fTemp6 * (0.0 - fTemp7)) / (fTemp7 + 1.0)))) * fTemp4));
		fRec1[1] = fRec1[0];
		fRec0[1] = fRec0[0];
		fRec3[1] = fRec3[0];
		fRec2[1] = fRec2[0];
	}
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *input1, FAUSTFLOAT *output0, FAUSTFLOAT *output1, Dsp *p)
{
	p->compute(count, input0, input1, output0, output1);
}


void Dsp::connect(uint32_t port,void* data)
{
	switch ((PortIndex)port)
	{
	default:
		break;
	}
}

void Dsp::connect_static(uint32_t port,void* data, Dsp *p)
{
	p->connect(port, data);
}


Dsp *plugin() {
	return new Dsp();
}

void Dsp::del_instance(Dsp *p)
{
	delete p;
}

/*
typedef enum
{
} PortIndex;
*/

} // end namespace compressor
