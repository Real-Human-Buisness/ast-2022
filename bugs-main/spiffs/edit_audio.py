
from pathlib import Path
from pydub import AudioSegment
import subprocess

folder_name = "data_1"
dry_run = False

if __name__ == "__main__":
    pathlist = Path(f"./{folder_name}").glob('*.wav')
    for path in pathlist:
        actual_file = path.resolve()
        print(f"formatting {actual_file}")
        stereo_audio = AudioSegment.from_file(actual_file, format="wav")
        stereo_audio = stereo_audio.set_frame_rate(22050)
        mono_audios = stereo_audio.split_to_mono()
        if not dry_run:
            mono_audios[0].export(actual_file,format="wav", codec='pcm_s16le')
        print(f"finished formatting {actual_file}")