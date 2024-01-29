package gpu

type memInfo struct {
	TotalMemory uint64 `json:"total_memory,omitempty"`
	FreeMemory  uint64 `json:"free_memory,omitempty"`
	DeviceCount uint32 `json:"device_count,omitempty"`
}

// Beginning of an `adam info` command
type GpuInfo struct {
	memInfo
	Library string `json:"library,omitempty"`

	// Optional variant to select (e.g. versions, cpu feature flags)
	Variant string `json:"variant,omitempty"`

	// TODO add other useful attributes about the card here for discovery information
}
