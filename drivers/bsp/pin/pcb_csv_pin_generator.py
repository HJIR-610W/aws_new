#!/usr/bin/env python3
"""
PCB Pin Map Header Generator for CSV files

This script parses pcb_x_pin.csv files and generates pcb_5_pin.h header files
with GPIO pin definitions. DI/DO functions get prefixes, others use original labels.

Usage:
    python pcb_csv_pin_generator.py <input_file.csv> [output_file.h]
    
Example:
    python pcb_csv_pin_generator.py drivers/bsp/pin/pcb_5_pin.csv drivers/bsp/pin/pcb_5_pin.h
"""

import sys
import os
import csv
from typing import List, Tuple, Optional

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

class PcbCsvPinGenerator:
    """Main class for generating PCB pin map headers from CSV files"""
    
    def __init__(self):
        self.pins: List[PinData] = []
        
    def parse_csv_file(self, file_path: str) -> None:
        """Parse the CSV pcb pin configuration file"""
        self.pins.clear()
        
        try:
            # Try different encodings for Korean text
            encodings = ['utf-8-sig', 'utf-8', 'cp949', 'euc-kr', 'latin-1']
            
            for encoding in encodings:
                try:
                    with open(file_path, 'r', encoding=encoding) as f:
                        # Read CSV file
                        csv_reader = csv.reader(f)
                        rows = list(csv_reader)
                    print(f"Successfully read CSV file with {encoding} encoding")
                    break
                except UnicodeDecodeError:
                    continue
            else:
                raise Exception("Could not read file with any supported encoding")
                
        except FileNotFoundError:
            raise FileNotFoundError(f"Input file not found: {file_path}")
        except Exception as e:
            raise Exception(f"Error reading CSV file {file_path}: {e}")
            
        print(f"CSV file has {len(rows)} rows")
        
        # Print header for debugging
        if rows:
            print(f"Header row: {rows[0]}")
        
        for row_num, row in enumerate(rows, 1):
            try:
                # Skip empty rows or header row
                if not row or not row[0] or row[0].startswith('포트'):
                    continue
                    
                # Skip rows that don't contain port.pin format
                if not ('PORT' in row[0] and '.' in row[0]):
                    continue
                
                # Ensure we have enough columns
                if len(row) < 6:
                    continue
                    
                port_pin = row[0].strip()
                label = row[1].strip()
                function = row[2].strip()
                pullup = row[3].strip()
                af = row[4].strip()
                do_init = row[5].strip()
                description = row[6].strip() if len(row) > 6 else ""
                
                # Debug print for first few entries
                if len(self.pins) < 5:
                    print(f"Debug row {row_num}: port_pin={port_pin}, label={label}, function={function}")
                
                # Parse port.pin (e.g., "PORTA.0")
                if '.' not in port_pin:
                    continue
                    
                port, pin = port_pin.split('.', 1)
                port = port.replace('PORT', '')  # Remove PORT prefix
                
                # Skip if essential fields are empty
                if not label or label in ['NULL', ''] or not port or not pin:
                    continue
                    
                pin_data = PinData(port, pin, label, function, pullup, af, do_init, description)
                self.pins.append(pin_data)
                
            except Exception as e:
                print(f"Warning: Error parsing row {row_num}: {e}")
                continue
                
        print(f"Parsed {len(self.pins)} pins from {file_path}")
        
    def generate_header_content(self) -> str:
        """Generate the complete header file content"""
        content = []
        content.append("#ifndef PCB_5_PIN_H")
        content.append("#define PCB_5_PIN_H")
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
            
        content.append("#endif /* PCB_5_PIN_H */")
        
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

def main():
    """Main function"""
    if len(sys.argv) < 2:
        print("Usage: python pcb_csv_pin_generator.py <input_file.csv> [output_file.h]")
        print("Example: python pcb_csv_pin_generator.py drivers/bsp/pin/pcb_5_pin.csv drivers/bsp/pin/pcb_5_pin.h")
        sys.exit(1)
        
    input_file = sys.argv[1]
    
    # Check if input file is CSV
    if not input_file.lower().endswith('.csv'):
        print("Error: Input file must be a CSV file (.csv)")
        sys.exit(1)
    
    # Generate output filename if not provided
    if len(sys.argv) >= 3:
        output_file = sys.argv[2]
    else:
        # Default: same directory as input, change extension to .h
        base_name = os.path.splitext(input_file)[0]
        output_file = f"{base_name}.h"
        
    generator = PcbCsvPinGenerator()
    
    try:
        print(f"Parsing CSV input file: {input_file}")
        generator.parse_csv_file(input_file)
        
        generator.print_summary()
        
        print(f"Generating output file: {output_file}")
        generator.generate_header_file(output_file)
        
        print("Generation completed successfully!")
        
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()