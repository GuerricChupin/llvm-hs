#define __STDC_LIMIT_MACROS
#include "llvm-c/Target.h"
#include "LLVM/Internal/FFI/LibFunc.h"
#include "LLVM/Internal/FFI/Target.h"
#include "LLVM/Internal/FFI/Target.hpp"
#include "llvm-c/Core.h"
#include "llvm-c/TargetMachine.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

namespace llvm {
// Taken from llvm/lib/Target/TargetMachineC.cpp
// These functions need to be marked as static to avoid undefined behavior
// due to multiple definitions
static LLVMTargetRef wrap(const Target *P) {
  return reinterpret_cast<LLVMTargetRef>(const_cast<Target *>(P));
}

static inline TargetLibraryInfoImpl *unwrap(LLVMTargetLibraryInfoRef P) {
  return reinterpret_cast<TargetLibraryInfoImpl*>(P);
}

static inline LLVMTargetLibraryInfoRef wrap(const TargetLibraryInfoImpl *P) {
  TargetLibraryInfoImpl *X = const_cast<TargetLibraryInfoImpl*>(P);
  return reinterpret_cast<LLVMTargetLibraryInfoRef>(X);
}

static FloatABI::ABIType unwrap(LLVM_Hs_FloatABI x) {
  switch (x) {
#define ENUM_CASE(x)                                                           \
  case LLVM_Hs_FloatABI_##x:                                              \
    return FloatABI::x;
    LLVM_HS_FOR_EACH_FLOAT_ABI(ENUM_CASE)
#undef ENUM_CASE
  default:
    return FloatABI::ABIType(0);
  }
}

static LibFunc unwrap(LLVMLibFunc x) {
  switch (x) {
#define ENUM_CASE(x)                                                           \
  case LLVMLibFunc__##x:                                                       \
    return LibFunc_ ## x;
    LLVM_HS_FOR_EACH_LIB_FUNC(ENUM_CASE)
#undef ENUM_CASE
  default:
    return LibFunc(0);
  }
}

static LLVMLibFunc wrap(LibFunc x) {
  switch (x) {
#define ENUM_CASE(x)                                                           \
  case LibFunc_ ## x:                                                             \
    return LLVMLibFunc__##x;
    LLVM_HS_FOR_EACH_LIB_FUNC(ENUM_CASE)
#undef ENUM_CASE
  default:
    return LLVMLibFunc(0);
  }
}

static LLVM_Hs_FloatABI wrap(FloatABI::ABIType x) {
  switch (x) {
#define ENUM_CASE(x)                                                           \
  case FloatABI::x:                                                            \
    return LLVM_Hs_FloatABI_##x;
    LLVM_HS_FOR_EACH_FLOAT_ABI(ENUM_CASE)
#undef ENUM_CASE
  default:
    return LLVM_Hs_FloatABI(0);
  }
}

static FPOpFusion::FPOpFusionMode unwrap(LLVM_Hs_FPOpFusionMode x) {
  switch (x) {
#define ENUM_CASE(x)                                                           \
  case LLVM_Hs_FPOpFusionMode_##x:                                        \
    return FPOpFusion::x;
    LLVM_HS_FOR_EACH_FP_OP_FUSION_MODE(ENUM_CASE)
#undef ENUM_CASE
  default:
    return FPOpFusion::FPOpFusionMode(0);
  }
}

static LLVM_Hs_FPOpFusionMode wrap(FPOpFusion::FPOpFusionMode x) {
  switch (x) {
#define ENUM_CASE(x)                                                           \
  case FPOpFusion::x:                                                          \
    return LLVM_Hs_FPOpFusionMode_##x;
    LLVM_HS_FOR_EACH_FP_OP_FUSION_MODE(ENUM_CASE)
#undef ENUM_CASE
  default:
    return LLVM_Hs_FPOpFusionMode(0);
  }
}
}

extern "C" {

LLVMBool LLVM_Hs_InitializeNativeTarget() {
  return LLVMInitializeNativeTarget() || InitializeNativeTargetAsmPrinter() ||
         InitializeNativeTargetAsmParser();
}

LLVMTargetRef LLVM_Hs_LookupTarget(const char *arch, const char *ctriple,
                                        const char **tripleOut,
                                        const char **cerror) {
  std::string error;
  Triple triple(ctriple);
  if (const Target *result =
          TargetRegistry::lookupTarget(arch, triple, error)) {
    *tripleOut = strdup(triple.getTriple().c_str());
    return wrap(result);
  }
  *cerror = strdup(error.c_str());
  return 0;
}

TargetOptions *LLVM_Hs_CreateTargetOptions() {
  TargetOptions *to = new TargetOptions();
  return to;
}

void LLVM_Hs_SetTargetOptionFlag(TargetOptions *to,
                                      LLVM_Hs_TargetOptionFlag f,
                                      unsigned v) {
  switch (f) {
#define ENUM_CASE(op)                                                          \
  case LLVM_Hs_TargetOptionFlag_##op:                                     \
    to->op = v ? 1 : 0;                                                        \
    break;
    LLVM_HS_FOR_EACH_TARGET_OPTION_FLAG(ENUM_CASE)
#undef ENUM_CASE
  }
}

static llvm::DebugCompressionType unwrap(LLVM_Hs_DebugCompressionType compressionType) {
    switch(compressionType) {
#define ENUM_CASE(op)                                \
        case LLVM_Hs_DebugCompressionType_ ## op:    \
            return llvm::DebugCompressionType::op;
        LLVM_HS_FOR_EACH_DEBUG_COMPRESSION_TYPE(ENUM_CASE)
#undef ENUM_CASE
    default:
            assert(false && "Unknown debug compression type");
        return llvm::DebugCompressionType::None;
    }
}

static LLVM_Hs_DebugCompressionType wrap(llvm::DebugCompressionType compressionType) {
    switch(compressionType) {
#define ENUM_CASE(op)                                   \
        case llvm::DebugCompressionType::op:            \
            return LLVM_Hs_DebugCompressionType_ ## op;
        LLVM_HS_FOR_EACH_DEBUG_COMPRESSION_TYPE(ENUM_CASE)
#undef ENUM_CASE
    default: {
            assert(false && "Unknown debug compression type");
            return LLVM_Hs_DebugCompressionType_None;
        }
    }
}

void LLVM_Hs_SetCompressDebugSections(TargetOptions *to,
                                      LLVM_Hs_DebugCompressionType compress) {
    to->CompressDebugSections = unwrap(compress);
}

LLVM_Hs_DebugCompressionType LLVM_Hs_GetCompressDebugSections(TargetOptions* to) {
    return wrap(to->CompressDebugSections);
}

unsigned LLVM_Hs_GetTargetOptionFlag(TargetOptions *to,
                                          LLVM_Hs_TargetOptionFlag f) {
  switch (f) {
#define ENUM_CASE(op)                                                          \
  case LLVM_Hs_TargetOptionFlag_##op:                                     \
    return to->op;
    LLVM_HS_FOR_EACH_TARGET_OPTION_FLAG(ENUM_CASE)
#undef ENUM_CASE
  default:
    assert(false && "Unknown target option flag");
    return 0;
  }
}

void LLVM_Hs_SetStackAlignmentOverride(TargetOptions *to, unsigned v) {
  to->StackAlignmentOverride = v;
}

unsigned LLVM_Hs_GetStackAlignmentOverride(TargetOptions *to) {
  return to->StackAlignmentOverride;
}

void LLVM_Hs_SetFloatABIType(TargetOptions *to, LLVM_Hs_FloatABI v) {
  to->FloatABIType = unwrap(v);
}

LLVM_Hs_FloatABI LLVM_Hs_GetFloatABIType(TargetOptions *to) {
  return wrap(to->FloatABIType);
}

void LLVM_Hs_SetAllowFPOpFusion(TargetOptions *to,
                                     LLVM_Hs_FPOpFusionMode v) {
  to->AllowFPOpFusion = unwrap(v);
}

LLVM_Hs_FPOpFusionMode LLVM_Hs_GetAllowFPOpFusion(TargetOptions *to) {
  return wrap(to->AllowFPOpFusion);
}

void LLVM_Hs_DisposeTargetOptions(TargetOptions *t) { delete t; }

// const TargetLowering *LLVM_Hs_GetTargetLowering(LLVMTargetMachineRef t)
// {
// 	return unwrap(t)->getTargetLowering();
// }

char *LLVM_Hs_GetDefaultTargetTriple() {
  return strdup(sys::getDefaultTargetTriple().c_str());
}

char *LLVM_Hs_GetProcessTargetTriple() {
  return strdup(sys::getProcessTriple().c_str());
}

const char *LLVM_Hs_GetHostCPUName(size_t &len) {
  StringRef r = sys::getHostCPUName();
  len = r.size();
  return r.data();
}

char *LLVM_Hs_GetHostCPUFeatures() {
  StringMap<bool> featureMap;
  std::string features;
  if (sys::getHostCPUFeatures(featureMap)) {
    bool first = true;
    for (llvm::StringMap<bool>::const_iterator it = featureMap.begin();
         it != featureMap.end(); ++it) {
      if (!first) {
        features += ",";
      }
      first = false;
      features += (it->second ? "+" : "-") + it->first().str();
    }
  }
  return strdup(features.c_str());
}

char *LLVM_Hs_GetTargetMachineDataLayout(LLVMTargetMachineRef t) {
  return strdup(
      unwrap(t)->createDataLayout().getStringRepresentation().c_str());
}

LLVMTargetLibraryInfoRef
LLVM_Hs_CreateTargetLibraryInfo(const char *triple) {
    const TargetLibraryInfoImpl* p = new TargetLibraryInfoImpl(Triple(triple));
    return wrap(p);
}

LLVMBool LLVM_Hs_GetLibFunc(
	LLVMTargetLibraryInfoRef l,
	const char *funcName,
	LLVMLibFunc *f
) {
	LibFunc func;
	LLVMBool result = unwrap(l)->getLibFunc(funcName, func);
	*f = wrap(func);
	return result;
}

const char *LLVM_Hs_LibFuncGetName(
	LLVMTargetLibraryInfoRef l,
	LLVMLibFunc f,
	size_t *nameSize
) {
	TargetLibraryInfo impl(*unwrap(l));
    StringRef s = impl.getName(unwrap(f));
	*nameSize = s.size();
	return s.data();
}

void LLVM_Hs_LibFuncSetAvailableWithName(
	LLVMTargetLibraryInfoRef l,
	LLVMLibFunc f,
	const char *name
) {
	unwrap(l)->setAvailableWithName(unwrap(f), name);
}

void LLVM_Hs_DisposeTargetLibraryInfo(LLVMTargetLibraryInfoRef l) {
	delete unwrap(l);
}

void LLVM_Hs_InitializeAllTargets() {
  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmPrinters();
  // None of the other components are bound yet
}

// This is identical to LLVMTargetMachineEmit but LLVM doesn’t expose this function so we copy it here.
LLVMBool LLVM_Hs_TargetMachineEmit(
    LLVMTargetMachineRef T,
    LLVMModuleRef M,
    raw_pwrite_stream *OS,
    LLVMCodeGenFileType codegen,
    char **ErrorMessage
) {
  TargetMachine* TM = unwrap(T);
  Module* Mod = unwrap(M);

  legacy::PassManager pass;

  std::string error;

  Mod->setDataLayout(TM->createDataLayout());

  TargetMachine::CodeGenFileType ft;
  switch (codegen) {
    case LLVMAssemblyFile:
      ft = TargetMachine::CGFT_AssemblyFile;
      break;
    default:
      ft = TargetMachine::CGFT_ObjectFile;
      break;
  }
  if (TM->addPassesToEmitFile(pass, *OS, ft)) {
    error = "TargetMachine can't emit a file of this type";
    *ErrorMessage = strdup(error.c_str());
    return true;
  }

  pass.run(*Mod);

  OS->flush();
  return false;
}

}
