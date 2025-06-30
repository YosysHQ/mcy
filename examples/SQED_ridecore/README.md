# This demo showcases Mutation Cover with Yosys (MCY)'s evaluation of Symbolic Quick Error Detection (SQED)'s ability to verify ridecore processor

[ridecore](https://github.com/ridecore/ridecore.git) is a dual-issue, six-stage, out-of-order CPU with support for up to 64 instructions in flight, two ALUs, one multiplier, and one load/store unit, aimed at high-performance applications. 

[SQED](https://github.com/upscale-project/generic-sqed-demo.git) is a transformative processors and hardware accelerators formal verification methodology that can check a design-independent universal property, reducing manual property development efforts. SQED employs bounded model checking (BMC) to formally verify the self-consistency property, which asserts that the execution of an original instruction and its duplicate produces the consistent outcome.   

