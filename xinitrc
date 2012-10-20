# .Xauthority file needs to be in home directory to have access to the X session
cp ~/.Xauthority /tmp/.xx
# Initialize decryption
/usr/bin/cryptsetup-gui chome
# Make sure device is decrypted
sleep .5
# Make working directory the newly mounted home directory
cd
# Restore .Xauthority
mv /tmp/.xx ~/.Xauthority
# Execute "real" .xinitrc
exec ~/.xinitrc
