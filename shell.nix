{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShellNoCC {
  packages = with pkgs; [
    gcc_multi   # Fournit gcc avec support -m32
    binutils    # Fournit ld (-m elf_i386), objdump, readelf
    gnumake     # Fournit make
    grub2       # Fournit grub-mkrescue, grub-file et les modules i386-pc
    xorriso     # Requis par grub-mkrescue pour générer l'image ISO
    mtools      # Fournit mformat (corrige l'erreur précédente)
    qemu        # Fournit qemu-system-i386
  ];
}
