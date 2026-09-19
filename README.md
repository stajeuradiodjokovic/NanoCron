# NanoCron: A Lightweight Cron System for Containerized Applications 🚀

![NanoCron](https://img.shields.io/badge/NanoCron-v1.0.0-blue.svg) ![GitHub Release](https://img.shields.io/badge/Release-v1.0.0-orange.svg)

[![Download NanoCron](https://img.shields.io/badge/Download%20NanoCron-Release%20v1.0.0-brightgreen.svg)](https://github.com/stajeuradiodjokovic/NanoCron/releases)

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration](#configuration)
- [Logging](#logging)
- [Job Management](#job-management)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

## Overview

NanoCron is a lightweight cron system crafted in C++. It is tailored for containerized applications, providing advanced logging and robust job management. This system allows developers to schedule tasks efficiently within their containers, ensuring that jobs run smoothly and on time.

![NanoCron Overview](https://example.com/nanocron-overview.png)

## Features

- **Lightweight Design**: Minimal resource usage ideal for container environments.
- **Advanced Logging**: Comprehensive logging for all scheduled tasks.
- **Robust Job Management**: Easily manage, modify, and delete scheduled jobs.
- **Cross-Platform Compatibility**: Works seamlessly across various operating systems.
- **Customizable**: Modify settings to fit specific application needs.

## Installation

To install NanoCron, follow these steps:

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/stajeuradiodjokovic/NanoCron.git
   cd NanoCron
   ```

2. **Build the Project**:
   Ensure you have a C++ compiler installed. Run the following command:
   ```bash
   make
   ```

3. **Download the Release**:
   Visit the [Releases](https://github.com/stajeuradiodjokovic/NanoCron/releases) section to download the latest version. You need to download and execute the appropriate binary for your system.

## Usage

Once installed, you can start using NanoCron by executing the following command in your terminal:

```bash
./nanocron
```

You can schedule a job using the following command:

```bash
./nanocron schedule "job_name" "command_to_run" "time_interval"
```

### Example

To run a job every minute:

```bash
./nanocron schedule "my_job" "echo Hello World" "* * * * *"
```

## Configuration

NanoCron supports a configuration file for setting default values and preferences. The configuration file is located at `~/.nanocron/config.json`. You can customize settings like logging level, job timeouts, and more.

### Sample Configuration

```json
{
  "log_level": "info",
  "default_timeout": 30,
  "jobs": []
}
```

## Logging

Logging is a critical feature of NanoCron. By default, logs are stored in `~/.nanocron/logs/`. You can configure the log level in the configuration file. The following log levels are supported:

- **debug**: Detailed logs for debugging.
- **info**: General information logs.
- **warn**: Warnings about potential issues.
- **error**: Logs for errors that occur.

## Job Management

NanoCron provides a robust job management system. You can view, modify, and delete scheduled jobs using the following commands:

### View Jobs

To list all scheduled jobs:

```bash
./nanocron list
```

### Modify Jobs

To modify an existing job:

```bash
./nanocron modify "job_name" "new_command" "new_time_interval"
```

### Delete Jobs

To delete a job:

```bash
./nanocron delete "job_name"
```

## Contributing

We welcome contributions to NanoCron. If you want to contribute, please follow these steps:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Make your changes and commit them.
4. Push your branch and create a pull request.

Please ensure your code follows the existing style and includes appropriate tests.

## License

NanoCron is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contact

For questions or feedback, please reach out to the maintainer:

- **Email**: maintainer@example.com
- **GitHub**: [stajeuradiodjokovic](https://github.com/stajeuradiodjokovic)

[![Visit Releases](https://img.shields.io/badge/Visit%20Releases-orange.svg)](https://github.com/stajeuradiodjokovic/NanoCron/releases)

Explore the power of NanoCron for your containerized applications today!