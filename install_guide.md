# Installation Guide

This guide details the steps to set up your environment and install necessary dependencies.

## Prerequisites
- Python 3.6+ and pip
- Git

## Setup Virtual Environment

1. Open your terminal and create a virtual environment:
   ```
   python3 -m venv env
   ```
2. Activate the environment:
   - On macOS/Linux:
     ```
     source env/bin/activate
     ```
   - On Windows:
     ```
     env\Scripts\activate
     ```

## Install Dependencies

1. Upgrade pip:
   ```
   pip install --upgrade pip
   ```
   sudo apt-get install python-smbus2
   ```
   git clone https://github.com/bluerobotics/ms5837-python
   ```
2. Install dependencies from requirements file:
   ```
   pip install -r requirements.txt
   ```

## Verifying the Installation

Run the main script to test:
```
python ssc_rov_float_final_v2.py
```

## Troubleshooting

- If you encounter issues with ms5837n, ensure Git is installed and confirm the repository URL and tag in requirements.txt.

## Contact

For further assistance, contact the development team.
