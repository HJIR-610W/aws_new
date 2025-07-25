#!/usr/bin/env python3
"""
PCB Pin Map Header Generator for Excel files

This script parses pcb_x_pin.xlsx files and generates pcb_pin_map.h header files
with GPIO pin definitions. DI/DO functions get prefixes, others use original labels.

Usage:
    python pcb_excel_pin_generator.py <input_file.xlsx> [output_file.h]
    
Example:
    python pcb_excel_pin_generator.py drivers/bsp/pin/pcb_5_pin.xlsx drivers/bsp/pin/pcb_pin_map.h

Requirements:
    pip install openpyxl pandas
"""

import sys
import os
from typing import List, Tuple, Optional
import pandas as pd

class PinData:
    """Represents a single pin configuration"""
    def __init__(self, port: str, pin: str, label: str, function: str, pullup: str, af: str, do_init: str, description: str = ""):
        self.port = port.upper()
        self.pin = pin
        self.label = label
        self.function = function.upper()
        self.pullup = pullup
        self.af = af
        self.do_init = do_init
        self.description = description
        
    def get_pin_name(self) -> str:
        """Generate pin name with appropriate prefix for DI/DO functions"""
        if self.function in ['DI', 'DO']:
            return f"{self.function}_{self.label}"
        else:
            return self.label
            
    def get_gpio_pin_number(self) -> str:
        """Convert pin number to GPIO_PIN_x format"""
        return f"GPIO_PIN_{self.pin}"
        
    def get_gpio_port(self) -> str:
        """Convert port to GPIO port format"""
        return f"GPIO{self.port}"

class PcbExcelPinGenerator:
    """Main class for generating PCB pin map headers from Excel files"""
    
    def __init__(self):
        self.pins: List[PinData] = []
        
    def parse_excel_file(self, file_path: str) -> None:
        """Parse the Excel pcb pin configuration file"""
        self.pins.clear()
        
        try:
            # Read Excel file
            df = pd.read_excel(file_path, engine='openpyxl')
            print(f"Successfully read Excel file: {file_path}")
            print(f"Excel shape: {df.shape}")
            print(f"Columns: {list(df.columns)}")
            
        except FileNotFoundError:
            raise FileNotFoundError(f"Input file not found: {file_path}")
        except Exception as e:
            raise Exception(f"Error reading Excel file {file_path}: {e}")
            
        # Print first few rows for debugging
        print("First 5 rows:")
        print(df.head())
        
        for index, row in df.iterrows():
            try:
                # Skip empty rows or header-like rows
                if pd.isna(row.iloc[0]) or str(row.iloc[0]).startswith('포트'):
                    continue
                    
                # Get values from each column
                port_pin = str(row.iloc[0]).strip() if not pd.isna(row.iloc[0]) else ""
                label = str(row.iloc[1]).strip() if not pd.isna(row.iloc[1]) else ""
                function = str(row.iloc[2]).strip() if not pd.isna(row.iloc[2]) else ""
                pullup = str(row.iloc[3]).strip() if not pd.isna(row.iloc[3]) else ""
                af = str(row.iloc[4]).strip() if not pd.isna(row.iloc[4]) else ""
                do_init = str(row.iloc[5]).strip() if not pd.isna(row.iloc[5]) else ""
                description = str(row.iloc[6]).strip() if len(row) > 6 and not pd.isna(row.iloc[6]) else ""
                
                # Debug print for first few entries
                if len(self.pins) < 5:
                    print(f"Debug row {index}: port_pin={port_pin}, label={label}, function={function}")
                
                # Skip if port_pin doesn't contain '.'
                if '.' not in port_pin or not port_pin.startswith('PORT'):
                    continue
                    
                # Parse port.pin (e.g., "PORTA.0")
                port, pin = port_pin.split('.', 1)
                port = port.replace('PORT', '')  # Remove PORT prefix
                
                # Skip if essential fields are empty
                if not label or label in ['NULL', 'nan'] or not port or not pin:
                    continue
                    
                pin_data = PinData(port, pin, label, function, pullup, af, do_init, description)
                self.pins.append(pin_data)
                
            except Exception as e:
                print(f"Warning: Error parsing row {index}: {e}")
                continue
                
        print(f"Parsed {len(self.pins)} pins from {file_path}")
        
    def generate_header_content(self) -> str:
        """Generate the complete header file content"""
        content = []
        content.append("#ifndef PCB_PIN_MAP_H")
        content.append("#define PCB_PIN_MAP_H")
        content.append("")
        content.append("#include \"stm32f4xx_hal.h\"")
        content.append("")
        
        # Group pins by port
        ports = {}
        for pin in self.pins:
            if pin.port not in ports:
                ports[pin.port] = []
            ports[pin.port].append(pin)
            
        # Sort ports alphabetically
        for port in sorted(ports.keys()):
            content.append(f"// PORT{port} pins")
            
            # Sort pins by pin number within each port
            port_pins = sorted(ports[port], key=lambda x: int(x.pin))
            
            for pin in port_pins:
                pin_name = pin.get_pin_name()
                gpio_pin = pin.get_gpio_pin_number()
                gpio_port = pin.get_gpio_port()
                
                # Generate pin definition
                pin_define = f"#define {pin_name}_Pin"
                port_define = f"#define {pin_name}_GPIO_Port"
                
                # Pad for alignment
                pin_define = pin_define.ljust(40)
                port_define = port_define.ljust(40)
                
                content.append(f"{pin_define} {gpio_pin}")
                content.append(f"{port_define} {gpio_port}")
                
            content.append("")
            
        content.append("#endif /* PCB_PIN_MAP_H */")
        
        return "\n".join(content)
        
    def generate_header_file(self, output_path: str) -> None:
        """Generate and write the header file"""
        content = self.generate_header_content()
        
        # Create directory if it doesn't exist
        output_dir = os.path.dirname(output_path)
        if output_dir:
            os.makedirs(output_dir, exist_ok=True)
        
        try:
            with open(output_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"Generated header file: {output_path}")
        except Exception as e:
            raise Exception(f"Error writing file {output_path}: {e}")
            
    def print_summary(self) -> None:
        """Print summary of parsed pins"""
        if not self.pins:
            print("No pins parsed")
            return
            
        print(f"\nSummary:")
        print(f"Total pins: {len(self.pins)}")
        
        # Count by function
        functions = {}
        for pin in self.pins:
            func = pin.function
            if func not in functions:
                functions[func] = 0
            functions[func] += 1
            
        print("Functions:")
        for func, count in sorted(functions.items()):
            print(f"  {func}: {count}")

def check_dependencies():
    """Check if required packages are installed"""
    try:
        import pandas
        import openpyxl
        return True
    except ImportError as e:
        print(f"Missing required package: {e}")
        print("Please install required packages:")
        print("pip install pandas openpyxl")
        return False

def main():
    """Main function"""
    if not check_dependencies():
        sys.exit(1)
        
    if len(sys.argv) < 2:
        print("Usage: python pcb_excel_pin_generator.py <input_file.xlsx> [output_file.h]")
        print("Example: python pcb_excel_pin_generator.py drivers/bsp/pin/pcb_5_pin.xlsx drivers/bsp/pin/pcb_pin_map.h")
        sys.exit(1)
        
    input_file = sys.argv[1]
    
    # Check if input file is Excel
    if not input_file.lower().endswith(('.xlsx', '.xls')):
        print("Error: Input file must be an Excel file (.xlsx or .xls)")
        sys.exit(1)
    
    # Generate output filename if not provided
    if len(sys.argv) >= 3:
        output_file = sys.argv[2]
    else:
        # Default: same directory as input, change extension to .h
        base_name = os.path.splitext(input_file)[0]
        output_file = f"{base_name}_pin_map.h"
        
    generator = PcbExcelPinGenerator()
    
    try:
        print(f"Parsing Excel input file: {input_file}")
        generator.parse_excel_file(input_file)
        
        generator.print_summary()
        
        print(f"Generating output file: {output_file}")
        generator.generate_header_file(output_file)
        
        print("Generation completed successfully!")
        
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()