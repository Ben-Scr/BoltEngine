namespace Bolt
{
    /// <summary>
    /// Lightweight reference to a texture asset by file path.
    /// Use with [ShowInEditor] to allow drag-drop assignment in the Inspector.
    /// </summary>
    public struct TextureRef
    {
        public string Path;

        public bool IsValid => !string.IsNullOrEmpty(Path);

        public TextureRef(string path) { Path = path; }

        public override string ToString() => Path ?? "(None)";
    }

    /// <summary>
    /// Lightweight reference to an audio asset by file path.
    /// Use with [ShowInEditor] to allow drag-drop assignment in the Inspector.
    /// </summary>
    public struct AudioRef
    {
        public string Path;

        public bool IsValid => !string.IsNullOrEmpty(Path);

        public AudioRef(string path) { Path = path; }

        public override string ToString() => Path ?? "(None)";
    }
}
