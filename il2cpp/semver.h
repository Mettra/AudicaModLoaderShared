#pragma once

struct semver {
	uint32_t major;
	uint32_t minor;
	uint32_t patch;

	bool operator< (const semver &rhs) {
		if (major != rhs.major) {
			return major < rhs.major;
		}

		if (minor != rhs.minor) {
			return minor < rhs.minor;
		}

		if (patch != rhs.patch) {
			return patch < rhs.patch;
		}

		return false;
	}
};
