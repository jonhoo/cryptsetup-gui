# .Xauthority file needs to be in home directory to have access to the X session
cp ~/.Xauthority /tmp/.xx
# Enable mouse cursor in X without WM
# https://github.com/johnlane/xplane-utils/blob/master/doc/OneMansWayToConfigureXPlane.md#mouse-cursor
if [ -e /usr/bin/xsetroot ]; then
  xsetroot -cursor_name left_ptr
fi
# Initialize decryption
/usr/bin/cryptsetup-gui -m chome
# Make sure device is decrypted
sleep .5
# Make working directory the newly mounted home directory
cd
# Restore .Xauthority
mv /tmp/.xx ~/.Xauthority
# Execute "real" .xinitrc
exec ~/.xinitrc
