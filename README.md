**Table of Contents:**
- [Computing FLOPs with Intel Software Development Emulator (Intel SDE)](#computing-flops-with-intel-software-development-emulator-intel-sde)
- [What You Get](#what-you-get)
- [Getting Started](#getting-started)
  * [FLOPs of the Entire Application](#flops-of-the-entire-application)
  * [FLOPs of Selected Sections](#flops-of-selected-sections)
  * [User Specified Profiling Files](#user-specified-profiling-files)
  * [MPI and OpenMP](#mpi-and-openmp)
- [Caveats](#caveats)
- [Validation](#validation)
- [TODO](#todo)
- [Contact](#contact)
- [Acknowledgements](#acknowledgements)
- [License](#license)

# Computing FLOPs with Intel Software Development Emulator (Intel SDE)
This project hosts the Python script `intel_sde_flops.py` to compute the number of Floating Point OPerations (FLOPs) executed by any application, entirely or for selected sections within the application. The script is based on the article [Calculating “FLOP” using Intel® Software Development Emulator (Intel® SDE)](https://software.intel.com/en-us/articles/calculating-flop-using-intel-software-development-emulator-intel-sde) which describes how to manually compute the FLOPs. Since it is a tedious manual task, we have automatized it by a Python script.

# What You Get
* Compute FLOPs from either closed or known source application
* FLOPs are counted for either the entire application or sections -- only the latter requires an annotation of sources
* Support of all Intel Architecture SIMD extensions (SSE2-4.2, AVX, AVX2 and all flavors of AVX512)
* Support of future (unreleased) Intel Architectures
* Emulation of modern architectures on older systems -- e.g. run AVX512 Skylake applications on a Haswell system
* Support for FMA instruction set extensions (counted as two operations)
* Separation of single and double precision FLOPs
* Listing FLOPs by threads
* Awareness of masking instructions (AVX512)
* The script works for both Python 2.x and 3.x
* Should also work on AMD processors (untested: please give feedback!)

New with version 1.1:
 * ~~AVX-512_4FMAPS: 4x FMA for single precision FP (Ice Lake server `-icx`)~~  
   ~~Note: Current Intel SDE (8.50) does not support this yet!~~  
   Erratum: Seems AVX-512_4FMAPS is not planned for Intel Core/Xeon processors. This was only in Intel Xeon Phi Processor (KNM: Knights Mill), which was never released to market.
 * AVX-512_BF16: bfloat16 dot product (Cooper Lake `-cpx`)

New with version 1.2 (experimental):
 * Added computation of arithmetic intensity (AI := FLOPs/Bytes accessed)

# Getting Started
We show two examples. The frist demonstrates how to collect the FLOPs count of the entire application which can be closed source. The second shows how to control which sections of the code should be subject of counting FLOPs.  
In both cases the [Intel Software Development Emulator (Intel SDE)](https://software.intel.com/en-us/articles/intel-software-development-emulator) is needed. Please download the latest version and upack it (latest version as of writing is [`sde-external-9.0.0-2021-11-07-lin.tar.xz`](https://www.intel.com/content/www/us/en/download/684897/intel-software-development-emulator.html)).  
Intel SDE is the executable `sde64` found in the top level directory within the tar ball. We assume your application of interest is `app` whose number of executed floating point operations should be counted.

## FLOPs of the Entire Application
Execute `app` within Intel SDE using the following invocation:  
  
`$ sde64 -iform -mix -dyn_mask_profile -- ./app`  
  
This creates two files `sde-mix-out.txt` and `sde-dyn-mask-profile.txt` in the working directory. The former contains information about the type and number of instructions executed. The latter provides information about whether elements on SIMD vectors have been masked (only for AVX512).  
To calculate the FLOPs of the application, execute the script within the same working directory:  
  
`$ python intel_sde_flops.py`  
  
It uses both files created by Intel SDE and shows the FLOPs separated by single/double precision, for all application thread IDs (TID), and summarized over all threads.  
  
**Example**:
    
    $ sde64 -iform -mix -dyn_mask_profile -- ./app
    <output of app>
    $ python intel_sde_flops.py
    Version: 1.1
    TID: 0 (OS-TID: 28764):
        Unmasked single prec. FLOPs: 0
        Masked single prec. FLOPs: 0
        Unmasked double prec. FLOPs: 102453126
        Masked double prec. FLOPs: 81940800
        Instructions executed: 75126024
        FMA instructions executed: 10242600
        Total bytes written: 92196963
        Total bytes read: 92196963
        Arithmetic intensity (approx.): 1.0 (EXPERIMENTAL)
    TID: 1 (OS-TID: 28770):
        Unmasked single prec. FLOPs: 0
        Masked single prec. FLOPs: 0
        Unmasked double prec. FLOPs: 51200065
        Masked double prec. FLOPs: 40960000
        Instructions executed: 37448333
        FMA instructions executed: 5120000
        Total bytes written: 46080002
        Total bytes read: 46080063
        Arithmetic intensity (approx.): 1.0 (EXPERIMENTAL)
    =============================================
    Sum:
        Single prec. FLOPs: 0
        Double prec. FLOPs: 276553991
        Total instructions executed: 112574357
        Total FMA instructions executed: 15362600
        Total bytes written: 138276965
        Total bytes read: 138277026
        Total arithmetic intensity (approx.): 1.0 (EXPERIMENTAL)

        
In the example, the application `app` only used instructions operating on double precision floating point values. Thread 0 executed 102453126 double precision FLOPs which were unmasked, i.e. the operations (instructions) were either using scalars or the entire length of SIMD vectors/registers. Furthermore, 81940800 double precision FLOPs were computed using masked operations, which are operations on SIMD registers selecting only a subset of elements to which an individual operation is applied. Since only AVX512 has masked instructions<sup>1</sup> so far, we can also conclude that `app` was compiled for one of the AVX512 flavors. Thread 1 only executed unmasked 51200065 double precision FLOPs. The sum of all double precision FLOPs is shown at the end of the output (276553991).  
It furthermore shows the number of overall instructions executed by thread (e.g. 75126024 for thread 0) or entirely over all threads (112574357). Also the number of explicit FMA instructions are shown the same way (e.g. 10242600 for thread 0). Note that this is only the count of the FMA instructions and not how many individual operations have been carried out. Those vary depending on how many elements are on a vector processed by an FMA instruction or whether masking was used.  
  
<sup>1</sup>: The Many Integrated Core (MIC) Architecture was the first Intel Architecture with masked instructions. However, Intel SDE cannot emulate MIC applications.  
  
**Note**:  
By default, Intel SDE (`sde64`) defaults to the processor on which it is executed. It is also possible to emulate another/newer processors (instruction sets) by the following options (see `sde64 --help`):
  
     -quark  Set chip-check and CPUID for Intel(R) Quark CPU
     -p4     Set chip-check and CPUID for Intel(R) Pentium4 CPU
     -p4p    Set chip-check and CPUID for Intel(R) Pentium4 Prescott CPU
     -mrm    Set chip-check and CPUID for Intel(R) Merom CPU
     -pnr    Set chip-check and CPUID for Intel(R) Penryn CPU
     -nhm    Set chip-check and CPUID for Intel(R) Nehalem CPU
     -wsm    Set chip-check and CPUID for Intel(R) Westmere CPU
     -snb    Set chip-check and CPUID for Intel(R) Sandy Bridge CPU
     -ivb    Set chip-check and CPUID for Intel(R) Ivy Bridge CPU
     -hsw    Set chip-check and CPUID for Intel(R) Haswell CPU
     -bdw    Set chip-check and CPUID for Intel(R) Broadwell CPU
     -slt    Set chip-check and CPUID for Intel(R) Saltwell CPU
     -slm    Set chip-check and CPUID for Intel(R) Silvermont CPU
     -glm    Set chip-check and CPUID for Intel(R) Goldmont CPU
     -glp    Set chip-check and CPUID for Intel(R) Goldmont Plus CPU
     -tnt    Set chip-check and CPUID for Intel(R) Tremont CPU
     -skl    Set chip-check and CPUID for Intel(R) Skylake CPU
     -cnl    Set chip-check and CPUID for Intel(R) Cannon Lake CPU
     -icl    Set chip-check and CPUID for Intel(R) Ice Lake CPU
     -skx    Set chip-check and CPUID for Intel(R) Skylake server CPU
     -clx    Set chip-check and CPUID for Intel(R) Cascade Lake CPU
     -cpx    Set chip-check and CPUID for Intel(R) Cooper Lake CPU
     -icx    Set chip-check and CPUID for Intel(R) Ice Lake server CPU
     -knl    Set chip-check and CPUID for Intel(R) Knights landing CPU
     -knm    Set chip-check and CPUID for Intel(R) Knights mill CPU
     -tgl    Set chip-check and CPUID for Intel(R) Tiger Lake CPU
     -adl    Set chip-check and CPUID for Intel(R) Alder Lake CPU
     -spr    Set chip-check and CPUID for Intel(R) Sapphire Rapids CPU
     -future Set chip-check and CPUID for Intel(R) Future chip CPU


E.g., to emulate an Icelake server CPU, invoke Intel SDE like this:  
  
`$ sde64 -icx -iform -mix -dyn_mask_profile -- ./app`

## FLOPs of Selected Sections
To restrict the FLOPs counting to specific sections within an application place the following markers in the source code:
* `__SSC_MARK(0xFACE);`  
  Start/resume collecting FLOPs past this line.
* `__SSC_MARK(0xDEAD);`  
  Stop/pause collecting FLOPs.

The markers are defined as **internal intrinsics by the Intel C++ Compiler**. If you use GCC, LLVM/Clang, or other C/C++ compilers, add the following preprocessor makro at the beginning of your source files to add support for the markers:

    #ifndef __SSC_MARK
    #define __SSC_MARK(tag)                                                        \
            __asm__ __volatile__("movl %0, %%ebx; .byte 0x64, 0x67, 0x90 "         \
                                 ::"i"(tag) : "%ebx")
    #endif

Recompile the application and execute it with Intel SDE like this:  
  
`$ sde64 -iform -mix -dyn_mask_profile -start_ssc_mark FACE:repeat -stop_ssc_mark DEAD:repeat -- ./app`  
  
Again, it will leave two files in the current working directory (`sde-mix-out.txt` and `sde-dyn-mask-profile.txt`). Execute the Python script in the same working directory to extract the number of FLOPs executed within the section(s):  
  
`$ python intel_sde_flops.py`  
  
**Example**:

    $ cat app.c
    ...
    __SSC_MARK(0xFACE);
    for(int i = 0; i < N; ++i) {
        ...
    }
    __SSC_MARK(0xDEAD);
    ...
    $ gcc app.c -o app
    $ sde64 -iform -mix -dyn_mask_profile -start_ssc_mark FACE:repeat -stop_ssc_mark DEAD:repeat -- ./app
    <output of app>
    $ python intel_sde_flops.py
    TID: 0 (OS-TID: 28190):
        Unmasked single prec. FLOPs: 0
        Masked single prec. FLOPs: 0
        Unmasked double prec. FLOPs: 102400000
        Masked double prec. FLOPs: 81920000
        Instructions executed: 69380000
        FMA instructions executed: 10240000
        Total bytes written: 92196963
        Total bytes read: 92196963
        Arithmetic intensity (approx.): 1.0 (EXPERIMENTAL)
    TID: 1 (OS-TID: 28197):
        Unmasked single prec. FLOPs: 0
        Masked single prec. FLOPs: 0
        Unmasked double prec. FLOPs: 51200000
        Masked double prec. FLOPs: 40960000
        Instructions executed: 34690000
        FMA instructions executed: 5120000
        Total bytes written: 46080002
        Total bytes read: 46080063
        Arithmetic intensity (approx.): 1.0 (EXPERIMENTAL)
    =============================================
    Sum:
        Single prec. FLOPs: 0
        Double prec. FLOPs: 276480000
        Total instructions executed: 104070000
        Total FMA instructions executed: 15360000
        Total bytes written: 138276965
        Total bytes read: 138277026
        Total arithmetic intensity (approx.): 1.0 (EXPERIMENTAL)

In the example, one loop of the application `app` was annotated with the start/stop markers. Hence only FLOPs executed within the loop are measured.  
  
**Note**:  
The markers can be placed arbitrarily in the source code. However, it is recommended not to nest them.  
  
**Note for using markers with other languages**:  
* Python:  
For using markers within any Python script, refer to the instructions within the [`for_python`](./for_python/) subdirectory.
* Fortran:  
For using markers within any Fortran application, refer to the instructions within the [`for_fortran`](./for_fortran/) subdirectory.
  
All follow the same principles as to using markers directly in C/C++.

## User Specified Profiling Files
Instead of using the default files `sde-mix-out.txt` and `sde-dyn-mask-profile.txt` in the local working directory, the script also accepts user specified files:  
`$ python intel_sde_flops.py [<sde_mix_out> <sde_dyn_mask_profile>]`  

If no arguments are used, defaults are:
* `<sde_mix_out>`: `sde-mix-out.txt`
* `<sde_dyn_mask_profile>`: `sde-dyn-mask-profile.txt`

If arguments are used, specify both in correct order:
* `<sde_mix_out> <sde_dyn_mask_profile>`

The files `<sde_mix_out>` and `<sde_dyn_mask_profile>` are created by Intel SDE's `-mix -iform` and `-dyn_mask_profile` options, respectively.

## MPI and OpenMP
If an MPI enabled application should be analyzied (on a shared file system), use option `-i`. This ensures that all files generated have individual file names, containing the process ID. Alternatively, option `-odir` can be used to specify separate output directories for every node/rank.

For OpenMP enabled applications, consider the option `-global_region`. This option ensures that all threads are included in the analysis.

# Caveats
When using Intel SDE for counting the FLOPs, be aware of the following pecularities when using the results:

1. **What is considered a floating point operation (FLOP)?**  
  Intel SDE is working on an instruction level. It hence regards an instruction, which operates on one floating point value, as a single FLOP (with the exception of FMAs -- see below). FLOPs are computations of different kinds but no memory stores or loads. Also any [type conversions](https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=convert) between different precision FP types or from/to integer types are not counted as FLOPs. We might add dedicated counters for conversion operations in the future to give a summary if such operations have significant impact on the runtime (diagnostic).  
  Computations are defined by the instructions of the underlying architecture, with the most common ones being addition, subtraction, multiplication and division. There are also instructions for computing the reciprocal, reciprocal square root, exponential, etc. There's a limited amount of possible operations given by the instruction set of the processor's micro-architecture.  
  Higher level operations as expressed in high level languages like C/C++ or Fortran are mapped by compilers to individual or a set of instructions. This results in two problems:
    1. **Simple high level operations are more complex than they seem:**  
    Let's consider a division expressed in C++, like so: `c = a/b;`  
    This actually ends up being two instructions for the Intel architectures carried out by compiler optimizations:  
    `b` is turned into the reciprocal first, which is then multiplied with `a`.  
    The reason behind this is not the lack of a dedicated division instruction -- it naively could have been compiled that way. The division, however, is a complex instruction which can take tenth of cycles to compute (e.g. see throughput of an [AVX division using double precision floating point](https://software.intel.com/sites/landingpage/IntrinsicsGuide/#techs=AVX&text=div&expand=2129,2126,2126) in the Intel Intrinsics Guide). Computing the [reciprocal](https://software.intel.com/sites/landingpage/IntrinsicsGuide/#techs=AVX&text=rcp&expand=2129,2126,2126,4450,2161,4450) and a [multiplication](https://software.intel.com/sites/landingpage/IntrinsicsGuide/#techs=AVX&text=mult&expand=2129,2126,2126,4450,2161) to "emulate" the division is faster (approx. 4 vs. 10 cycles for Skylake).  
    Effects like this are caused by compiler optimizations. They are desired and make applications more efficient, but substitute even simple operations by multiple instructions.
    2. **Complex high level operations have no instruction counterpart:**  
    More common than i. are high level operations to which no single instruction in the processor micro-architecture exists. Examples are the exponent operation (e.g. for AVX<sup>2</sup>), or simple arithmetic on complex numbers. For the latter, the compiler would separate real and imaginary to apply individual instructions onto. For the former, implementations from math libraries are used (e.g. `libm`, Intel's svml, GCC's `libmvec`, etc.). See 3. below for more information on numerical libaries.  
    As a result, those implementations yield to additional instructions being executed to perform a single high level operation.
       
2. **How are contracted instructions (so-called FMA) handled?**  
   Contracted instructions combine multiple (high level) operations in one. Most common examples are fused multiply add (FMA) instructions which multiply two operands and add another one. Also a fused multiply subtract (FMS) exists, which subtracts instead of adding an operand. They are also referred to as "FMA", exploiting the fact that an FMS is a FMA with toggled sign bit of the operand.  
   Those instructions need special care as simply counting the instruction execution is not enough. Intel SDE offers different ways to extract information about usage of such contracted instructions. Our script is aware about that and properly counts them as two FLOPs.  
   Despite the proper detection and counting of FMAs, there is a side-effect when porting an application from an architecture without FMA support (e.g. AVX) to one which supports it (e.g. AVX2). As FMAs combine two operations in one instruction, the number of total instructions exectuted by an application is expected to be lower compared to an architecture without FMA capabilities. This can be used to study the effects (and potential) of FMAs to the application of interest. The more the overal executed instructions are reduced the more the application might benefit from FMAs. For this, two compilations of an application are needed. One with the FMA instruction set and one without FMA. For the latter one could still use the instruction set from the former compilation but tell the compiler to not create FMA instructions. Options would be `-mno-fma` (GCC or LLVM/Clang), or `-no-fma` (Intel).
   
3. **Numerical libraries can skew your results.**  
   Similar to caveat 1., II. from above, using 3rd party math libraries makes it hard to compute the correct FLOPs. The reason lies in the implementation which is typically not known. Also the expectation of what a FLOP actually is might be different -- is a FLOP a BLAS level 1 operation, or level 2 or 3?  
   Using such 3rd party math libraries can either be explicit or implicit. In cases the programmer uses such libraries explicitly, e.g. with OpenBLAS, ATLAS, or Intel MKL, functions are called directly which helps to recognize their scope. Implicit usage, however, is induced by the compiler itself in replacing `libm` functions by compiler specific runtime libraries, e.g. Intel's svml or GCC's `libmvec`. Even worse, depending on the SIMD instruction set available, 3rd party math libraries might decide during runtime of the application which optimized code paths to use.  
   We recommend to exclude 3rd party math libraries from computation of FLOPs. This can be done using the markers as shown above.

3. **Masking of operations for pre-AVX512 influences the counted FLOPs.**  
   AVX512 has masking of operands integrated for every vector operation, including load/store instructions. This is handled by the output of the `-dyn_mask_profile` option's output file, whenever SIMD extensions with AVX512 (and later) are used. For pre-AVX512 SIMD extensions there is no such integrated masking in the instruction set architecture. As a result, for pre-AVX512, compilers "emulate" masking of operands by using an additional vector register (of Booleans) for every operation to keep information of which element to mask out. This mask vector is then blended with the operation's result to receive a vector with only the elements of interest set.  
   As a consequence, this requires additional vector instructions to be generated by the compiler and executed during runtime, which increases the FLOPs count. Also it is indistinguishable at runtime where such masking was used by the compiler and which elements have been masked out. Hence, for pre-AVX512, Intel SDE will always deliver FLOPs of the full vectors for both the operation of interest and the emulated masking instructions.

4. **AVX-512_BF16 (bfloat16) operations are counted as single precision FLOPs.**  
  The current AVX-512_BF16 instructions comprise type converts (single precision FP to bfloat16) and dot products with bfloat16 operands (DPBF16). The type converts are not counted as FLOPs, but DPBF16 operations are. However, DPBF16 internally convert to single precision FP. As a result, DPBF16 operations are listed in the single precision (masked/unmasked) listings.

<sup>2</sup> An *exponent* instruction does not exist for SIMD extensions other than AVX512ER. Which could have side-effects when migrating from AVX to AVX512/AVX512ER.

# Validation
To ensure proper counting over changes of the script and different Intel SDE versions, the [`test`](./test/) subdirectory contains "unit tests" for validation purposes. Follow the instructions in the source file, which also documents the expected counts.

# TODO
The following future SIMD instruction sets still need to be validated:
* [AVX512_VNNI](https://software.intel.com/sites/landingpage/IntrinsicsGuide/#expand=3492,3488,2197,6,2179&avx512techs=AVX512_VNNI): These instructions operate on integer data types. It would require the script to also count integer operations (IOPS) which currently is not implemented.  
* [AMX-BF16](https://software.intel.com/sites/landingpage/IntrinsicsGuide/#amxtechs=AMXBF16): Intel Advanced Matrix Extensions (Intel AMX) which also operates on BF16 types. Curently this is only a tiled BF16 dot product.
Unit test was added but Intel SDE does not yet count the single FP (or BF16) operations. After contacting Intel, this might be added in a future release.

# Contact
Should you have any feedback or questions, please contact the author: Georg Zitzlsberger (georg.zitzlsberger(a)vsb.cz).

# Acknowledgements
This work was supported by The Ministry of Education, Youth and Sports from the Large Infrastructures for Research, Experimental Development and Innovations project ”IT4Innovations National Supercomputing Center – LM2015070”.

# License
This project is made available under the GNU General Public License, version 3 (GPLv3).
