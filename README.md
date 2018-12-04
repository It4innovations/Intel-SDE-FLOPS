# Computing FLOPs with Intel Software Development Emulator (Intel SDE)
This project hosts a Python script to compute the number of Floating Point OPerations (FLOPs) executed by any application, entirely or for selected sections within the application. The script is based on the article [Calculating “FLOP” using Intel® Software Development Emulator (Intel® SDE)](https://software.intel.com/en-us/articles/calculating-flop-using-intel-software-development-emulator-intel-sde) which describes how to manually compute the FLOPs. Since it is a tedious manual task, we have automatized it by a Python script.

# What You Get
* Compute FLOPs from either closed or known source application
* FLOPs are counted for either the entire application or sections -- only the latter requires an annotation of sources
* Support of all Intel Architecture SIMD extensions (SSE2-4.2, AVX, AVX2 and all flavors of AVX512)
* Support for FMA instruction set extensions (counted as two operations)
* Separation of single and double precision FLOPs
* Listing FLOPs by threads
* Awareness of masking instructions (AVX512)

# Getting Started
We show two examples. The frist demonstrates how to collect the FLOPs count of the entire application which can be closed source. The second shows how to control which sections of the code should be subject of counting FLOPs.\
In both cases the [Intel Software Development Emulator (Intel SDE)](https://software.intel.com/en-us/articles/intel-software-development-emulator) is needed. Please download the latest version and upack it (latest version as of writing is [`sde-external-8.16.0-2018-01-30-lin.tar.bz2`](https://software.intel.com/protected-download/267266/144917)). Intel SDE is the executable `sde64` found in the top level directory within the tar ball. We assume your application of interest is `app` whose number of executed floating point operations should be counted.

## FLOPs of the Entire Application
Execute `app` within Intel SDE using the following invocation:\
`$ sde64 -iform -mix -dyn_mask_profile -- ./app`\
This creates two files `sde-mix-out.txt` and `sde-dyn-mask-profile.txt` in the working directory. The former contains information about the type and number of instructions executed. The latter provides information about whether elements on SIMD vectors have been masked (only for AVX512).\
To calculate the FLOPs of the application, execute the script within the same working directory:\
`$ intel_sde_flops.py`
It uses both files created by Intel SDE and show the FLOPs separated by single/double precision and application thread IDs (TID).\
\
**Example**:\
    
    $ sde64 -iform -mix -dyn_mask_profile -- ./app
    <output of app>
    $ intel_sde_flops.py
    TID: 0 (OS-TID: 19116):
        Unmasked single prec. FLOPs: 0
        Masked single prec. FLOPs: 0
        Unmasked double prec. FLOPs: 184384663
        Masked double prec. FLOPs: 20800
    TID: 1 (OS-TID: 19122):
        Unmasked single prec. FLOPs: 0
        Masked single prec. FLOPs: 0
        Unmasked double prec. FLOPs: 92160103
        Masked double prec. FLOPs: 0
    =============================================
    Sum:
        Single prec. FLOPs: 0
        Double prec. FLOPs: 276565566

**Note**:\
By default, Intel SDE (`sde64`) defaults to the processor on which it is executed. It is also possible to emulate another/newer processors (instruction sets) by the following options (see `sde64 --help`):

    -mrm  Set chip-check and CPUID for Intel(R) Merom CPU
    -pnr  Set chip-check and CPUID for Intel(R) Penryn CPU
    -nhm  Set chip-check and CPUID for Intel(R) Nehalem CPU
    -wsm  Set chip-check and CPUID for Intel(R) Westmere CPU
    -snb  Set chip-check and CPUID for Intel(R) Sandy Bridge CPU
    -ivb  Set chip-check and CPUID for Intel(R) Ivy Bridge CPU
    -hsw  Set chip-check and CPUID for Intel(R) Haswell CPU
    -bdw  Set chip-check and CPUID for Intel(R) Broadwell CPU
    -slt  Set chip-check and CPUID for Intel(R) Saltwell CPU
    -slm  Set chip-check and CPUID for Intel(R) Silvermont CPU
    -glm  Set chip-check and CPUID for Intel(R) Goldmont CPU
    -skl  Set chip-check and CPUID for Intel(R) Skylake CPU
    -skx  Set chip-check and CPUID for Intel(R) Skylake server CPU
    -cnl  Set chip-check and CPUID for Intel(R) Cannonlake CPU
    -knl  Set chip-check and CPUID for Intel(R) Knights landing CPU
    -knm  Set chip-check and CPUID for Intel(R) Knights mill CPU

E.g., to emulate a Cannonlake CPU invoke Intel SDE like this:\
`$ sde64 -cnl -iform -mix -dyn_mask_profile -- app`

## FLOPs of an Selected Sections
To restrict the FLOPs counting to specific sections within an application place the following markers in the source code:
* `__SSC_MARK(0xFACE);`\
  Start/resume collecting FLOPs past this line.
* `__SSC_MARK(0xDEAD);`\
  Stop/pause collecting FLOPs.
  
Recompile the application and execute it with Intel SDE like this:\
`$ sde64 -iform -mix -dyn_mask_profile -start_ssc_mark FACE:repeat -stop_ssc_mark DEAD:repeat -- ./app`\
Again, it will leave two files in the current working directory (`sde-mix-out.txt` and `sde-dyn-mask-profile.txt`). Execute the Python script in the same working directory to extract the number of FLOPs executed within the section(s):\
`$ intel_sde_flops.py`\
\
**Example**:\

    $ cat app.c
    ...
    __SSC_MARK(0xFACE);
    for(int i = 0; i < N; ++i) {
    ...
    __SSC_MARK(0xDEAD);
    ...
    $ gcc app.c -o app
    $ sde64 -iform -mix -dyn_mask_profile -start_ssc_mark FACE:repeat -stop_ssc_mark DEAD:repeat -- ./app
    <output of app>
    $ intel_sde_flops.py
    TID: 0 (OS-TID: 19116):
        Unmasked single prec. FLOPs: 0
        Masked single prec. FLOPs: 0
        Unmasked double prec. FLOPs: 184384663
        Masked double prec. FLOPs: 20800
    TID: 1 (OS-TID: 19122):
        Unmasked single prec. FLOPs: 0
        Masked single prec. FLOPs: 0
        Unmasked double prec. FLOPs: 92160103
        Masked double prec. FLOPs: 0
    =============================================
    Sum:
        Single prec. FLOPs: 0
        Double prec. FLOPs: 276565566
