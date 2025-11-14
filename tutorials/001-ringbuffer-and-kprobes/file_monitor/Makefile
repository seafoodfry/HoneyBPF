.DEFAULT_GOAL := help

.PHONY: help
help:  ## Display this help message
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m<target>\033[0m\n"} /^[a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)


##@ Local Development.
.PHONY: vmlinux
vmlinux:  ## Create a vmlinux.h file
	bpftool btf dump file /sys/kernel/btf/vmlinux format c > src/bpf/vmlinux.h

.PHONY: build
build:  ## Build the program
	cargo fmt
	cargo build --release

.PHONY: run
run:  ## Run the program
	sudo ./target/release/file_monitor

##@ Development Cleanup.
.PHONY: clean
clean:  ## Delete build artifacts
	cargo clean