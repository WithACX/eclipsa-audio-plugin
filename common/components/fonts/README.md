# Fonts

The plugins' UI asks for Audiowide (the renderer header) and Roboto (all other
labels) by family name. These TTFs are compiled into both plugins as the
`font_data` binary-data target, so the labels render correctly on machines
that do not have the fonts installed. `EclipsaLookAndFeel` serves them.

Only the Regular weight ships, because no call site asks for another weight
or style. Add a TTF here, list it in `CMakeLists.txt`, and map it in
`EclipsaLookAndFeel` before a call site asks for a new weight.

## Sources and licenses

Both fonts are licensed under the SIL Open Font License 1.1. Keep each
`*-OFL.txt` next to its font, and keep the matching section of
`third_party/THIRD_PARTY_LICENSES.txt` current, because the installers ship
that file.

- Audiowide Regular: `ofl/audiowide` in
  [google/fonts](https://github.com/google/fonts).
- Roboto Regular: `hinted/static/Roboto-Regular.ttf` from the
  [roboto-3-classic v3.016 release](https://github.com/googlefonts/roboto-3-classic/releases/tag/v3.016);
  license text from `ofl/roboto` in google/fonts.
