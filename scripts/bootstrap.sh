#!/usr/bin/env bash
set -euo pipefail

BINUTILS_VERSION="2.42"
GCC_VERSION="13.2.0"
TARGET="i386-elf"
NPROC=$(nproc 2>/dev/null || echo 4)

BASE_DIR="$(pwd)/bootstrap"
SRC_DIR="${BASE_DIR}/src"
PREFIX="${BASE_DIR}"

usage() {
  cat <<EOF
Usage: $(basename "$0") [--clean] [--help]

  --clean    Remove build trees (bootstrap/src/build-*) before building.
             Downloaded tarballs and extracted source trees are kept.
  --help     Show this message.

Builds an i386-elf bare-metal cross-compiler into bootstrap/bin/.
  binutils  ${BINUTILS_VERSION}
  gcc       ${GCC_VERSION}
EOF
  exit 0
}

CLEAN=0
for arg in "$@"; do
  case "${arg}" in
    --help|-h) usage ;;
    --clean)   CLEAN=1 ;;
    *) echo "error: unknown argument: ${arg}" >&2; usage ;;
  esac
done

MISSING_CMDS=()
MISSING_PKGS=()

need_cmd() {
  local cmd="$1" pkg="$2"
  if ! command -v "${cmd}" >/dev/null 2>&1; then
    MISSING_CMDS+=("command: ${cmd}")
    MISSING_PKGS+=("${pkg}")
  fi
}

# Check for a C header by scanning standard include paths.
need_header() {
  local header="$1" pkg="$2"
  local found=0
  for dir in \
    /usr/include \
    /usr/local/include \
    /usr/include/x86_64-linux-gnu \
    /usr/include/aarch64-linux-gnu
  do
    [ -f "${dir}/${header}" ] && found=1 && break
  done
  if [ "${found}" -eq 0 ]; then
    MISSING_CMDS+=("header: <${header}>")
    MISSING_PKGS+=("${pkg}")
  fi
}

need_cmd wget       wget
need_cmd tar        tar
need_cmd make       build-essential
need_cmd gcc        build-essential
need_cmd g++        build-essential
need_cmd m4         m4
need_cmd bison      bison
need_cmd flex       flex
need_cmd makeinfo   texinfo

need_header gmp.h   libgmp-dev
need_header mpfr.h  libmpfr-dev
need_header mpc.h   libmpc-dev

if [ "${#MISSING_PKGS[@]}" -gt 0 ]; then
  echo "error: missing required tools/libraries:" >&2
  for item in "${MISSING_CMDS[@]}"; do
    echo "  ${item}" >&2
  done
  # Deduplicate package names for the one-liner install hint.
  UNIQUE_PKGS=$(printf '%s\n' "${MISSING_PKGS[@]}" | sort -u | tr '\n' ' ')
  echo "" >&2
  echo "Install with:" >&2
  echo "  sudo apt install ${UNIQUE_PKGS}" >&2
  exit 1
fi

if ! { [ -f /usr/include/isl/version.h ] || \
       [ -f /usr/local/include/isl/version.h ]; }; then
  echo "note: libisl-dev not found. GCC will build without Graphite loop optimizations."
  echo "      (optional: sudo apt install libisl-dev)"
fi

START_TIME=$(date +%s)

log() { echo "[$(date '+%H:%M:%S')] $*"; }

download() {
  local url="$1" dest="$2"
  if [ ! -f "${dest}" ]; then
    log "downloading $(basename "${dest}") ..."
    wget -q --show-progress -O "${dest}" "${url}"
  else
    log "already downloaded: $(basename "${dest}")"
  fi
}

elapsed() {
  local secs=$(( $(date +%s) - START_TIME ))
  printf '%dm%02ds' $(( secs / 60 )) $(( secs % 60 ))
}

if [ "${CLEAN}" -eq 1 ]; then
  log "removing old build trees ..."
  rm -rf "${SRC_DIR}/build-binutils" "${SRC_DIR}/build-gcc"
  log "clean done."
fi

mkdir -p "${SRC_DIR}"

BINUTILS_TARBALL="${SRC_DIR}/binutils-${BINUTILS_VERSION}.tar.gz"
BINUTILS_SRC="${SRC_DIR}/binutils-${BINUTILS_VERSION}"
BINUTILS_BUILD="${SRC_DIR}/build-binutils"

download \
  "https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.gz" \
  "${BINUTILS_TARBALL}"

if [ ! -d "${BINUTILS_SRC}" ]; then
  log "extracting binutils ..."
  tar -xf "${BINUTILS_TARBALL}" -C "${SRC_DIR}"
fi

if [ ! -f "${BINUTILS_BUILD}/.installed" ]; then
  log "configuring binutils ..."
  mkdir -p "${BINUTILS_BUILD}"
  (
    cd "${BINUTILS_BUILD}"
    "${BINUTILS_SRC}/configure" \
      --target="${TARGET}" \
      --prefix="${PREFIX}" \
      --with-sysroot \
      --disable-nls \
      --disable-werror
    log "building binutils (using ${NPROC} jobs) ..."
    make -j"${NPROC}"
    make install
    touch .installed
  )
  log "binutils installed. (elapsed: $(elapsed))"
else
  log "binutils already built. Skipping!"
fi

GCC_TARBALL="${SRC_DIR}/gcc-${GCC_VERSION}.tar.gz"
GCC_SRC="${SRC_DIR}/gcc-${GCC_VERSION}"
GCC_BUILD="${SRC_DIR}/build-gcc"

download \
  "https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz" \
  "${GCC_TARBALL}"

if [ ! -d "${GCC_SRC}" ]; then
  log "extracting gcc ..."
  tar -xf "${GCC_TARBALL}" -C "${SRC_DIR}"
fi

if [ ! -f "${GCC_BUILD}/.installed" ]; then
  log "configuring gcc ..."
  mkdir -p "${GCC_BUILD}"
  # shellcheck disable=SC2030
  (
    export PATH="${PREFIX}/bin:${PATH}"
    cd "${GCC_BUILD}"
    "${GCC_SRC}/configure" \
      --target="${TARGET}" \
      --prefix="${PREFIX}" \
      --disable-nls \
      --enable-languages=c,c++ \
      --without-headers
    log "building gcc stage 1 (using ${NPROC} jobs) ..."
    make -j"${NPROC}" all-gcc
    log "building target libgcc (using ${NPROC} jobs) ..."
    make -j"${NPROC}" all-target-libgcc
    make install-gcc
    make install-target-libgcc
    touch .installed
  )
  log "gcc installed. (elapsed: $(elapsed))"
else
  log "gcc already built. Skipping!"
fi

log "verifying installed binaries ..."
# shellcheck disable=SC2031
export PATH="${PREFIX}/bin:${PATH}"

for tool in gcc g++ ld as; do
  bin="${TARGET}-${tool}"
  if ! command -v "${bin}" >/dev/null 2>&1; then
    echo "error: expected binary not found: ${bin}" >&2
    exit 1
  fi
  version=$("${bin}" --version 2>&1 | head -1)
  log "  OK  ${bin}: ${version}"
done

TOTAL=$(elapsed)
echo ""
echo "Cross-compiler ready in ${TOTAL}."
echo "'make build' now possible."
echo ""
