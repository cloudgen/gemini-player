#!/usr/local/bin/zsh
# Build gemini-player on macOS.
# PATH comes from the zsh always-on file. Tools are taken from /usr/local/bin.
# This file is not run by /bin/bash.

emulate -L zsh
setopt err_exit nounset

if [[ -z ${ZSH_VERSION:-} ]]; then
  print -u2 "[ERROR] build-mac.zsh requires zsh. Do not run it with /bin/bash."
  exit 1
fi

zshenv_file=${ZSHENV:-${HOME}/.zshenv}
if [[ ! -f ${zshenv_file} ]]; then
  print -u2 "[ERROR] ${zshenv_file} is missing. Put /usr/local/bin on PATH in that zshenv file."
  exit 1
fi
setopt no_err_exit
source ${zshenv_file}
source_status=$?
setopt err_exit nounset
if (( source_status != 0 )); then
  print -u2 "[WARN] ${zshenv_file} returned ${source_status}. The compilers below are still absolute paths under /usr/local/bin."
fi

typeset -a required
required=(/usr/local/bin/zsh /usr/local/bin/clang /usr/local/bin/clang++ /usr/local/bin/cmake /usr/local/bin/make)
for tool in ${required}; do
  if [[ ! -x ${tool} ]]; then
    print -u2 "[ERROR] ${tool} is missing. This build does not call /usr/bin/${tool:t}."
    exit 1
  fi
done

cd ${0:A:h}
/usr/local/bin/cmake -S . -B build-mac -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER=/usr/local/bin/clang \
  -DCMAKE_CXX_COMPILER=/usr/local/bin/clang++ \
  -DCMAKE_OBJC_COMPILER=/usr/local/bin/clang \
  -DCMAKE_OBJCXX_COMPILER=/usr/local/bin/clang++ \
  -DCMAKE_MAKE_PROGRAM=/usr/local/bin/make
/usr/local/bin/cmake --build build-mac
print "Built build-mac/gemini-player"
