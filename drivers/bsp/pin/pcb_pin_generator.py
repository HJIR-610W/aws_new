#!/usr/bin/env python3
"""
PCB Pin Map Header Generator

This script parses pcb_x_pin.txt files and generates pcb_pin_map.h header files
with GPIO pin definitions. DI/DO functions get prefixes, others use original labels.

Usage:
    python pcb_pin_generator.py <input_file> [output_file]
    
Example:
    python pcb_pin_generator.py drivers/bsp/pin/pcb_5_pin.txt drivers/bsp/pin/pcb_pin_map.h
"""

import sys
import re
import os
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

class PcbPinGenerator:
    """Main class for generating PCB pin map headers"""
    
    def __init__(self):
        self.pins: List[PinData] = []
        
    def parse_pin_file(self, file_path: str) -> None:
        """Parse the pcb pin configuration file"""
        self.pins.clear()
        
        try:
            # Try UTF-8 first, then try other encodings
            encodings = ['utf-8', 'cp949', 'euc-kr', 'latin-1']
            lines = None
            
            for encoding in encodings:
                try:
                    with open(file_path, 'r', encoding=encoding) as f:
                        lines = f.readlines()
                    print(f"Successfully read file with {encoding} encoding")
                    break
                except UnicodeDecodeError:
                    continue
                    
            if lines is None:
                raise Exception("Could not read file with any supported encoding")
                
        except FileNotFoundError:
            raise FileNotFoundError(f"Input file not found: {file_path}")
        except Exception as e:
            raise Exception(f"Error reading file {file_path}: {e}")
            
        for line_num, line in enumerate(lines, 1):
            line = line.strip()
            
            # Remove debug output
            
            # Skip empty lines and header line  
            if not line or line.startswith('포트.핀'):
                continue
                
            # Skip lines that don't contain port.pin format
            if not ('PORT' in line and '.' in line):
                continue
                
            try:
                # No need to remove prefix since file doesn't have line numbers
                # Just process the line directly
                
                # Split by comma and clean up
                parts = [part.strip() for part in line.split(',')]
                
                if len(parts) < 6:
                    continue
                    
                # Parse port.pin (e.g., "PORTA.0")
                port_pin = parts[0].strip()
                if '.' not in port_pin:
                    print(f"Debug: Skipping line {line_num}, no '.' in port_pin: {port_pin}")
                    continue
                    
                port, pin = port_pin.split('.', 1)
                port = port.replace('PORT', '')  # Remove PORT prefix
                
                # Remove debug output
                
                label = parts[1].strip()
                function = parts[2].strip()
                pullup = parts[3].strip()
                af = parts[4].strip()
                do_init = parts[5].strip()
                description = parts[6].strip() if len(parts) > 6 else ""
                
                # Skip if label is empty or invalid
                if not label or label in ['NULL', ''] or not port or not pin:
                    continue
                    
                pin_data = PinData(port, pin, label, function, pullup, af, do_init, description)
                self.pins.append(pin_data)
                
            except Exception as e:
                print(f"Warning: Error parsing line {line_num}: {line}")
                print(f"Error: {e}")
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

def main():
    """Main function"""
    if len(sys.argv) < 2:
        print("Usage: python pcb_pin_generator.py <input_file> [output_file]")
        print("Example: python pcb_pin_generator.py drivers/bsp/pin/pcb_5_pin.txt drivers/bsp/pin/pcb_pin_map.h")
        sys.exit(1)
        
    input_file = sys.argv[1]
    
    # Generate output filename if not provided
    if len(sys.argv) >= 3:
        output_file = sys.argv[2]
    else:
        # Default: same directory as input, change extension to .h
        base_name = os.path.splitext(input_file)[0]
        output_file = f"{base_name}_pin_map.h"
        
    generator = PcbPinGenerator()
    
    try:
        print(f"Parsing input file: {input_file}")
        generator.parse_pin_file(input_file)
        
        generator.print_summary()
        
        print(f"Generating output file: {output_file}")
        generator.generate_header_file(output_file)
        
        print("Generation completed successfully!")
        
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()